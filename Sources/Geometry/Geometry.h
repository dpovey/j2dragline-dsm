#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "Module.h"
#include "Timestamp.h"

namespace j2 {

    const int MAX_GEOMETRY_JITTER_MS = 50;

    class GeometryConfig {
    public:
        double depth_adjust;
        double boom_angle;
        double boom_len;
        double beyond_boom_pt_rad;
        double drag_offset;
        double hoist_offset;
    };

    class Motion {
    public:
        double position() const;
        double velocity() const;
    };

    struct Point {
        double x;
        double y;
    };

    class Sheave {
    public:
        const Point& position() const;
    };

    /*
     *  Geometry class.  This class provides an abstract interface to an
     *  idealised dragline geometry model that can be used by application
     *  level modules such as Weighting, SIMS, and tightline.
     */

    struct Geometry
    {
        bool beyond_boom_pt;

        // Is the geometry valid
        bool valid;

        double drag_rope_angle;
        double drag_rope_len; // Rope length + chain length and bucket + optionally 4m
        double hoist_rope_angle;
        double hoist_rope_len;
        double bucket_height;
        double bucket_reach;
        double boom_angle;
        double boom_drag_angle;
        double boom_hoist_angle;
        double boom_length;
        double boom_foot_height;
        double boom_foot_distance;

        // Internal angles of ropes etc
        //double angle[10];
    };

    class GeometryManager : public Module {
    public:
        GeometryManager() {
            bind_value("/motion/hoist", &_hoist_motion);
            bind_value("/motion/drag", &_drag_motion);
            bind_value("/motion/swing", &_swing_motion);
            bind_value("/sheave/hoist", &_hoist_sheave);
            bind_value("/sheave/drag", &_drag_sheave);
            bind_value("/sensor/inclinometer/pitch", &_pitch);
            bind_value("/config/geometry", &_config);

            _set.add(_hoist_motion)
                .add(_drag_motion)
                .add(_swing_motion)
                .add(_hoist_sheave)
                .add(_drag_sheave)
                .add(_pitch);
        }

        void update();

    private:
        bool has_slack_rope() const {
            return fabs(_geometry->drag_rope_len - _geometry->hoist_rope_len) > _config.boom_len;
        }

        bool has_slack_hoist_rope() const {
            return has_slack_rope() && _geometry->drag_rope_len < _geometry->hoist_rope_len;
        }

        bool has_slack_drag_rope() const {
            return has_slack_rope() && _geometry->drag_rope_len > _geometry->hoist_rope_len;
        }

    private:
        TimestampedSet<MAX_GEOMETRY_JITTER_MS, boost::chrono::milliseconds> _set;
        Timestamped<Geometry> _geometry;
        GeometryConfig _config;
        Timestamped<Motion> _hoist_motion;
        Timestamped<Motion> _drag_motion;
        Timestamped<Motion> _swing_motion;
        Timestamped<Sheave> _hoist_sheave;
        Timestamped<Sheave> _drag_sheave;
        Timestamped<double> _pitch;
        bool _valid;
    };   

} // namespace j2

#endif // _GEOMETRY_H
