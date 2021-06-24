#include "graphicsitemresizer.h"
#include "resizehandleitem.h"
#include <QDebug>
#include <QPainter>
#include <QTransform>

static QSizeF handleSize = QSizeF(6, 6);
static QMarginsF handleBounds = QMarginsF(handleSize.width(), handleSize.height(), handleSize.width(), handleSize.height());

GraphicsItemResizer::GraphicsItemResizer(QGraphicsItem* target, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , mTarget(target)
{
    Q_ASSERT(mTarget);

    setFlag(ItemIsMovable);
    setFlag(ItemHasNoContents);

    mTarget->setParentItem(this);
    mTarget->setPos(handleBounds.left(), handleBounds.top());
    mTarget->setFlag(ItemIsMovable, false);

    QPen p(Qt::gray, 1, Qt::DashLine);
    p.setCosmetic(true);

    setBoundingRectAreaPen(p);
    setBoundingRectAreaBrush(QBrush(Qt::NoBrush));

    // sides
    mHandleItems.append(new HandleItem(HandleItem::Left, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Top, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Right, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Bottom, handleSize, this));
    // corners
    mHandleItems.append(new HandleItem(HandleItem::Top | HandleItem::Left, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Top | HandleItem::Right, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Bottom | HandleItem::Right, handleSize, this));
    mHandleItems.append(new HandleItem(HandleItem::Bottom | HandleItem::Left, handleSize, this));

    recalculate();
}

GraphicsItemResizer::~GraphicsItemResizer()
{

}

QBrush GraphicsItemResizer::handleItemBrush() const
{
    return mHandleItemBrush;
}

QPen GraphicsItemResizer::handleItemPen() const
{
    return mHandleItemPen;
}

QBrush GraphicsItemResizer::boundingRectAreaBrush() const
{
    return mBoundingRectAreaBrush;
}

QPen GraphicsItemResizer::boundingRectAreaPen() const
{
    return mBoundingRectAreaPen;
}

bool GraphicsItemResizer::boundingRectAreaVisible() const
{
    return mBoundingRectAreaVisible;
}

QRectF GraphicsItemResizer::boundingRect() const
{
    return mBounds;
}

void GraphicsItemResizer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (boundingRectAreaVisible())
    {
        painter->setPen(boundingRectAreaPen());
        painter->setBrush(boundingRectAreaBrush());
        painter->drawRect(mBounds - handleBounds);
    }
}

void GraphicsItemResizer::setHandleItemPen(const QPen &pen)
{
    if (mHandleItemPen != pen)
    {
        mHandleItemPen = pen;

        for (HandleItem *i : mHandleItems)
            i->setPen(pen);
    }
}

void GraphicsItemResizer::setHandleItemBrush(const QBrush &brush)
{
    if (mHandleItemBrush != brush)
    {
        mHandleItemBrush = brush;

        for (HandleItem *i : mHandleItems)
            i->setBrush(brush);
    }
}

void GraphicsItemResizer::setBoundingRectAreaBrush(const QBrush &brush)
{
    if (mBoundingRectAreaBrush != brush)
    {
        mBoundingRectAreaBrush = brush;
        update();
    }
}

void GraphicsItemResizer::setBoundingRectAreaPen(const QPen &pen)
{
    if (mBoundingRectAreaPen != pen)
    {
        mBoundingRectAreaPen = pen;
        update();
    }
}

void GraphicsItemResizer::setBoundingRectAreaVisible(bool visible)
{
    if (mBoundingRectAreaVisible != visible)
    {
        mBoundingRectAreaVisible = visible;
        setFlag(ItemHasNoContents, !mBoundingRectAreaVisible);
        update();
    }
}

void GraphicsItemResizer::recalculate()
{   
    auto rect = targetBoundingRect();
    auto diff = rect.topLeft() - QPointF(handleBounds.left(), handleBounds.top());

    updateBoundingRectSize((QRectF(QPointF(), rect.size()) + handleBounds).size());

    mTarget->setPos(mTarget->pos() - diff);
}

QGraphicsItem *GraphicsItemResizer::target() const
{
    return mTarget;
}

QRectF GraphicsItemResizer::targetBoundingRect() const
{
    return mapFromItem(mTarget, mTarget->shape()).boundingRect();
}

bool GraphicsItemResizer::handlersIgnoreTransformations() const
{
    return mHandlersIgnoreTransformations;
}

void GraphicsItemResizer::setHandlersIgnoreTransformations(bool ignore)
{
    if (mHandlersIgnoreTransformations != ignore)
    {
        mHandlersIgnoreTransformations = ignore;

        for (auto handleItem : mHandleItems)
            handleItem->setFlag(ItemIgnoresTransformations, ignore);
    }
}

void GraphicsItemResizer::updateHandleItemPositions()
{
    QRectF innerRect = mBounds - handleBounds;

    for (HandleItem *i : mHandleItems)
        i->targetRectChanged(innerRect);
}

void GraphicsItemResizer::updateBoundingRectSize(const QSizeF& size)
{
    if (mBounds.size() != size)
    {
        prepareGeometryChange();
        mBounds = QRectF(QPointF(), size);
        updateHandleItemPositions();
    }
}
