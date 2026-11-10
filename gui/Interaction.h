#pragma once
#include <QQuickWindow>
#include <QQuickItem>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QCursor>
#include <algorithm>
#include <cmath>

// Keep hit testing in the same coordinate system as the barrel-distortion shader.
class Interaction : public QObject {
    QQuickWindow* window;
    bool forwarding = false;
    QCursor hand, pointing, beam;
    int lastKind = -1;
    static QCursor cursor(int kind) {
        QPixmap pix(48,48); pix.fill(Qt::transparent); QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing); p.setPen(QPen(QColor("#203f52"),2)); p.setBrush(QColor("#78aaa0"));
        if(kind==2) {
            p.drawRect(21,5,6,37); p.drawRect(13,4,22,5); p.drawRect(13,39,22,5);
            return QCursor(pix,24,24);
        }
        QPolygon shape;
        if(kind==1) shape << QPoint(9,29)<<QPoint(9,5)<<QPoint(13,2)<<QPoint(17,5)<<QPoint(17,22)<<QPoint(22,19)<<QPoint(27,22)<<QPoint(31,21)<<QPoint(36,25)<<QPoint(41,27)<<QPoint(40,37)<<QPoint(34,45)<<QPoint(18,45)<<QPoint(4,31)<<QPoint(4,27);
        else shape << QPoint(8,25)<<QPoint(8,15)<<QPoint(13,12)<<QPoint(17,17)<<QPoint(19,10)<<QPoint(24,9)<<QPoint(27,16)<<QPoint(31,13)<<QPoint(36,16)<<QPoint(38,25)<<QPoint(42,29)<<QPoint(39,39)<<QPoint(32,45)<<QPoint(17,43)<<QPoint(4,31)<<QPoint(4,26);
        p.drawPolygon(shape); p.setPen(QPen(QColor("#bad5b4"),2)); p.drawLine(19,31,32,35);
        return QCursor(pix,kind==1?13:8,kind==1?3:24);
    }
    int kindAt(QQuickItem* item,QPointF pos) {
        if(!item->isVisible() || !item->isEnabled() || item->objectName()=="curvedScreen") return -1;
        auto local=item->mapFromScene(pos);
        if(item->clip() && !item->contains(local)) return -1;
        auto children=item->childItems();
        std::stable_sort(children.begin(),children.end(),[](auto a,auto b){return a->z()<b->z();});
        for(auto i=children.rbegin();i!=children.rend();++i) { int k=kindAt(*i,pos); if(k>=0) return k; }
        if(!item->contains(local)) return -1;
        if(item->inherits("QQuickTextInput")) return 2;
        if(item->inherits("QQuickAbstractButton") || item->inherits("QQuickComboBox") || item->inherits("QQuickSlider")) return 1;
        if(item->inherits("QQuickMouseArea") && item->property("cursorShape").toInt()==Qt::PointingHandCursor) return 1;
        return -1;
    }
public:
    static QPointF sourcePoint(QPointF pos,QSizeF size) {
        double scale=std::min(size.width()/1280.0,size.height()/800.0);
        QPointF origin((size.width()-1280*scale)/2,(size.height()-800*scale)/2);
        QPointF p((pos.x()-origin.x())/(1280*scale)*2-1,(pos.y()-origin.y())/(800*scale)*2-1);
        double factor=1+0.045*(p.x()*p.x()+p.y()*p.y());
        return origin+QPointF((p.x()*factor+1)*640*scale,(p.y()*factor+1)*400*scale);
    }
    static QPointF displayPoint(QPointF source,QSizeF size) {
        double scale=std::min(size.width()/1280.0,size.height()/800.0);
        QPointF origin((size.width()-1280*scale)/2,(size.height()-800*scale)/2);
        QPointF q((source.x()-origin.x())/(640*scale)-1,(source.y()-origin.y())/(400*scale)-1);
        double r=std::hypot(q.x(),q.y()),v=r;
        for(int i=0;i<8;++i) v-=(v+0.045*v*v*v-r)/(1+0.135*v*v);
        double k=r>0?v/r:1;
        return origin+QPointF((q.x()*k+1)*640*scale,(q.y()*k+1)*400*scale);
    }
    explicit Interaction(QQuickWindow* w):QObject(w),window(w),hand(cursor(0)),pointing(cursor(1)),beam(cursor(2)) {
        window->installEventFilter(this); QGuiApplication::setOverrideCursor(hand);
    }
    ~Interaction() override { QGuiApplication::restoreOverrideCursor(); }
    bool eventFilter(QObject*,QEvent* e) override {
        if(forwarding) return false;
        if(e->type()!=QEvent::MouseMove && e->type()!=QEvent::MouseButtonPress && e->type()!=QEvent::MouseButtonRelease && e->type()!=QEvent::MouseButtonDblClick) return false;
        auto mouse=static_cast<QMouseEvent*>(e); QPointF pos=mouse->position();
        bool warp=window->property("curvatureInput").toBool() && !window->property("modalOpen").toBool();
        if(warp) pos=sourcePoint(pos,window->size());
        int kind=std::max(0,kindAt(window->contentItem(),pos));
        if(kind!=lastKind) { QGuiApplication::changeOverrideCursor(kind==2?beam:kind==1?pointing:hand); lastKind=kind; }
        if(!warp) return false;
        QMouseEvent mapped(mouse->type(),pos,pos,mouse->globalPosition(),mouse->button(),mouse->buttons(),mouse->modifiers(),mouse->pointingDevice());
        forwarding=true; QCoreApplication::sendEvent(window,&mapped); forwarding=false; e->setAccepted(mapped.isAccepted()); return true;
    }
};
