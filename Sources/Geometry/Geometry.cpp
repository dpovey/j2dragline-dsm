#include "Geometry.h"
#include <cmath>

using namespace j2;

// TODO: Constants file?
const double MIN_HOIST_ROPE_ANGLE = -5.0 * M_PI / 180.0;
const double GRAVITY = 9.80665;

void GeometryManager::update() {
    if (!_set.is_synchronized()) return;

    Geometry g;
    g.valid = true;
   
    // Compute real boom angle from configured angle and pitch from inclinometer
    g.boom_angle = _config.boom_angle - _pitch;
    double cos_thetaAdjusted = cos(g.boom_angle);

    g.drag_rope_len = _drag_motion->position() + _config.drag_offset;
    g.hoist_rope_len = _hoist_motion->position() + _config.hoist_offset;

    // check that the sum of the rope lengths is not less than the boom length
    if (_config.boom_len > g.drag_rope_len + g.hoist_rope_len) {
        g.valid = false;
        // TODO:
        //publish("/alarms/geometry/invalid", g);
        g.valid = false;
        return;
    }
 
    if (has_slack_drag_rope()) {
        // Guess geometry using minimum hoist rope angle and approximate drage rope length
        g.hoist_rope_angle = MIN_HOIST_ROPE_ANGLE;
        g.drag_rope_angle = 
            atan((_config.boom_len * sin(g.boom_angle) - g.hoist_rope_len) / 
                  (_config.boom_len * cos_thetaAdjusted));

        // TODO: Should this use the minimum hoist rope angle
        // Should we add g.hoist_rope_len * sin(MIN_HOIST_ROPE_ANGLE)?
        g.bucket_reach =
            _drag_sheave->position().x + _config.boom_len * cos_thetaAdjusted;

        g.bucket_height =
            _hoist_sheave->position().y - 
            g.hoist_rope_len * cos(g.hoist_rope_angle) + 
            _config.depth_adjust;

        g.beyond_boom_pt = true;
        return;
    } 

    if (has_slack_hoist_rope()) { 
        g.hoist_rope_angle = g.boom_angle / 2;
        g.drag_rope_angle = M_PI / 2;
        
        g.valid = false;
        
        g.bucket_reach = 
            _drag_sheave->position().x +
            g.drag_rope_len * cos(g.drag_rope_angle);
        g.bucket_height =
            _hoist_sheave->position().y -
            g.hoist_rope_len * cos(g.hoist_rope_angle)
            + _config.depth_adjust;
        g.beyond_boom_pt = true;
        return;
    } 

    g.boom_drag_angle = 
        acos(g.drag_rope_len*g.drag_rope_len + 
              _config.boom_len*_config.boom_len -
              (g.hoist_rope_len*g.hoist_rope_len)/
              (2.0*g.drag_rope_len*_config.boom_len));
    g.drag_rope_angle = g.boom_angle - g.boom_drag_angle;
    g.boom_hoist_angle =
        acos(g.hoist_rope_len*g.hoist_rope_len +
              _config.boom_len*_config.boom_len -
              (g.drag_rope_len*g.drag_rope_len)/
              (2.0*g.hoist_rope_len*_config.boom_len));
    g.hoist_rope_angle =
        M_PI/2 -
        g.boom_angle -
        g.boom_hoist_angle;
    
    g.hoist_rope_angle =
        std::max(g.hoist_rope_angle, MIN_HOIST_ROPE_ANGLE);
    
    g.bucket_reach =
        _drag_sheave->position().x +
        g.drag_rope_len * cos(g.drag_rope_angle);
    
    g.bucket_height =
        _hoist_sheave->position().y -
        g.hoist_rope_len * cos(g.hoist_rope_angle) +
        _config.depth_adjust;
    
    // WTF?
    double rot_vel = _swing_motion->velocity();
    double a_radial = g.bucket_reach * rot_vel * rot_vel;
    double beta_vertical = atan(a_radial/GRAVITY);
    g.beyond_boom_pt = (g.hoist_rope_angle < 
                                _config.beyond_boom_pt_rad - beta_vertical);

    _geometry = Timestamped<Geometry>(g, _set);
}
