#include "PointArrayCurveSegmentModel.hpp"
#include <iscore/serialization/VisitorCommon.hpp>

CurveSegmentModel*PointArrayCurveSegmentModel::clone(
        const Id<CurveSegmentModel>& id,
        QObject *parent) const
{
    auto cs = new PointArrayCurveSegmentModel{id, parent};
    cs->setStart(this->start());
    cs->setEnd(this->end());

    // Previous and following shall be set afterwards by the cloner.
    return cs;
}

QString PointArrayCurveSegmentModel::name() const
{
    return "PointArray";
}

void PointArrayCurveSegmentModel::serialize(const VisitorVariant& vis) const
{
    serialize_dyn(vis, *this);
}

void PointArrayCurveSegmentModel::on_startChanged()
{
    emit dataChanged();
}

void PointArrayCurveSegmentModel::on_endChanged()
{
    emit dataChanged();
}

void PointArrayCurveSegmentModel::updateData(int numInterp) const
{
    if(!m_valid)
    {
        m_data.clear();
        m_data.reserve(m_points.size());

        double length = max_x - min_x;
        double amplitude = max_y - min_y;
        // Scale all the points between 0 / 1 in <->
        // and the local min / max in vertical

        for(const auto& elt : m_points)
        {
            m_data.push_back(
            { (m_end.x() - m_start.x()) * (elt.first - min_x) / length + m_start.x(),
              (elt.second - min_y) / amplitude });
        }
    }
}

double PointArrayCurveSegmentModel::valueAt(double x) const
{
    ISCORE_TODO;
    return 0;
}

void PointArrayCurveSegmentModel::addPoint(double x, double y)
{
    // If x < start.x() or x > end.x(), we update start / end
    // The points must keep their apparent position.
    // If y < 0 or y > 1, we rescale everything (and update min / max)
    int s = m_points.size();

    if(y < min_y)
        min_y = y;
    if(y > max_y)
        max_y = y;

    if(s > 0)
    {
        if(x < min_x)
            min_x = x;
        else if(x > max_x)
            max_x = x;

        if(y < min_y)
            min_y = y;
        else if(y > max_y)
            max_y = y;
    }
    else
    {
        min_x = x;
        max_x = x;
        min_y = y;
        max_y = y;
    }

    m_points.insert(std::make_pair(x, y));

    m_valid = false;
    emit dataChanged();
}

std::vector<std::unique_ptr<LinearCurveSegmentModel> > PointArrayCurveSegmentModel::piecewise() const
{
    const auto& pts = data();
    std::vector<std::unique_ptr<LinearCurveSegmentModel> > vec;
    vec.reserve(pts.size() - 1);

    updateData(0);

    for(int i = 0; i < pts.size() - 1; i++)
    {
        auto cmd = std::make_unique<LinearCurveSegmentModel>(Id<CurveSegmentModel>(i), nullptr);
        cmd->setStart(pts[i]);
        cmd->setEnd(pts[i+1]);
    }

    return vec;
}
