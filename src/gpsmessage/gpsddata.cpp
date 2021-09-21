/* ========================= GPSD2eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2019 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= GPSD2eCAL LICENSE =================================
*/

#include "gpsddata.h"
#include "gpshelper.h"
#include <cmath>
#include "ecal/ecal_time.h"

using namespace utility;
using namespace gps_fix_constants;

//------------------------------------------------------------------------------
//  gpsddata:: gpsddata
//------------------------------------------------------------------------------
 gpsddata:: gpsddata(Idevice* device, const string &name):gpsmsg()
  , _gpsd_handler(device)
{
    _pub_gpsdata.Create(name);
}

//------------------------------------------------------------------------------
//  gpsddata::send_gps_message()
//------------------------------------------------------------------------------
void  gpsddata::send_gps_message()
{
    // read the new gps data
    _gps_data = update(_gpsd_handler);

    // set all gps data to proto struct
    set_gps_message();

    // publish message to ecal
    _pub_gpsdata.Send(_msg_gpsdata);
}

//------------------------------------------------------------------------------
// gpsddata::set_gps_gst()
//------------------------------------------------------------------------------
void gpsddata::set_gps_gst()
{
    /*
    * The structure describing the pseudorange errors (GPGST)
    */
    auto gst = new pb::gps::gpsdata_gps_gst_t();
    gst->set_alt_err_deviation     (_gps_data.gst.alt_err_deviation);
    gst->set_lat_err_deviation     (_gps_data.gst.lat_err_deviation);
    gst->set_lon_err_deviation     (_gps_data.gst.lon_err_deviation);
    gst->set_rms_deviation         (_gps_data.gst.rms_deviation);
    gst->set_smajor_deviation      (_gps_data.gst.smajor_deviation);
    gst->set_smajor_orientation    (_gps_data.gst.smajor_orientation);
    gst->set_sminor_deviation      (_gps_data.gst.sminor_deviation);

#if GPSD_API_MAJOR_VERSION < 9
    gst->set_utctime               (_gps_data.gst.utctime);
#else
    gst->set_utctime               (_gps_data.gst.utctime.tv_sec);
#endif
    _msg_gpsdata.set_allocated_gst_t(move(gst));
}

//------------------------------------------------------------------------------
//  gpsddata::set_gps_time()
//------------------------------------------------------------------------------
void  gpsddata::set_gps_dop()
{
    /* Dilution of precision factors */
    auto dop = new pb::gps::gpsdata_dop_t();
    dop->set_gdop       (_gps_data.dop.gdop);
    dop->set_hdop       (_gps_data.dop.hdop);
    dop->set_pdop       (_gps_data.dop.pdop);
    dop->set_tdop       (_gps_data.dop.tdop);
    dop->set_vdop       (_gps_data.dop.vdop);
    dop->set_xdop       (_gps_data.dop.xdop);
    dop->set_ydop       (_gps_data.dop.ydop);
    _msg_gpsdata.set_allocated_dop(move(dop));
}

//------------------------------------------------------------------------------
//  gpsddata::set_gps_time()
//------------------------------------------------------------------------------
void  gpsddata::set_gps_fix()
{    
    auto fix_data = new pb::gps::gpsdata_gps_fix_data();
    if(nullptr == fix_data) return;

    fix_data->set_altitude              (_gps_data.fix.altitude);
    fix_data->set_climb                 (_gps_data.fix.climb);
    fix_data->set_latitude              (_gps_data.fix.latitude);
    fix_data->set_epc                   (_gps_data.fix.epc);
    fix_data->set_epd                   (_gps_data.fix.epd);
    fix_data->set_eps                   (_gps_data.fix.eps);
    fix_data->set_epv                   (_gps_data.fix.epv);
    fix_data->set_epx                   (_gps_data.fix.epx);
    fix_data->set_epy                   (_gps_data.fix.epy);
    fix_data->set_longitude             (_gps_data.fix.longitude);
    fix_data->set_mode                  (_gps_data.fix.mode);
    fix_data->set_speed                 (_gps_data.fix.speed);
    fix_data->set_track                 (_gps_data.fix.track);
    fix_data->set_time                  (_gps_data.fix.time);
    _msg_gpsdata.set_allocated_fix_data(move(fix_data));

}

//------------------------------------------------------------------------------
//  gpsddata::set_gps_message()
//------------------------------------------------------------------------------
void  gpsddata::set_gps_message()
{
    set_gps_gst();
    set_gps_dop();
    set_gps_fix();
    set_gps_data();
    set_gps_header();
}

//------------------------------------------------------------------------------
// gpsddata::set_gps_data()
//------------------------------------------------------------------------------
void gpsddata::set_gps_data()
{
    _msg_gpsdata.set_satellites_used   (_gps_data.satellites_used);
    _msg_gpsdata.set_satellites_visible(_gps_data.satellites_visible);

#if GPSD_API_MAJOR_VERSION < 9
    _msg_gpsdata.set_epe               (_gps_data.epe);
    _msg_gpsdata.set_online            (_gps_data.online);
#else
    _msg_gpsdata.set_epe               (_gps_data.fix.eph);
    _msg_gpsdata.set_online            (_gps_data.online.tv_sec);
#endif
}

//------------------------------------------------------------------------------
// gpsddata::set_gps_data()
//------------------------------------------------------------------------------
void gpsddata::set_gps_header()
{
    // Set the timestamp
    auto header = new pb::Header();
    header->set_timestamp (eCAL::Time::GetMicroSeconds());
    _msg_gpsdata.set_allocated_header(move(header));
}