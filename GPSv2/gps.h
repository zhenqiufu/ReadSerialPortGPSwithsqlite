/*
 *************************************************
 *gps.h
 *function for read and transter the data to UTM
 *using serial lib to read the serial data
 *using GeographicLib to transfer to UTM
 *the header file can be read by C++ compilers
 *
 *by
 *************************************************
 */

#ifndef __GPS_H__
#define __GPS_H__

#include <string.h>
#include <GeographicLib/TransverseMercator.hpp>
#include <cstdio>
#include <exception>
#include <iomanip>
#include <iostream>

// OS Specific sleep
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"

#include <sqlite_modern_cpp.h>
using namespace sqlite;

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::fixed;
using std::setprecision;
using std::string;
using std::vector;

using namespace GeographicLib;
// Define a UTM projection for an arbitrary ellipsoid
class UTMalt {
 private:
  GeographicLib::TransverseMercator _tm;  // The projection
  double _lon0;  // Central longitude  double _lon0;            // Central
                 // longitude

  double _falseeasting, _falsenorthing;

 public:
  UTMalt(double a,  // equatorial radius
         double f,  // flattening
         int zone,  // the UTM zone + hemisphere
         bool northp)
      : _tm(a, f, Constants::UTM_k0()),
        _lon0(6 * zone - 183),
        _falseeasting(5e5),
        _falsenorthing(northp ? 0 : 100e5) {
    if (!(zone >= 1 && zone <= 60)) throw GeographicErr("zone not in [1,60]");
  }
  void Forward(double lat, double lon, double& x, double& y) {
    _tm.Forward(_lon0, lat, lon, x, y);
    x += _falseeasting;
    y += _falsenorthing;
  }
  void Reverse(double x, double y, double& lat, double& lon) {
    x -= _falseeasting;
    y -= _falsenorthing;
    _tm.Reverse(_lon0, x, y, lat, lon);
  }
};

void my_sleep(unsigned long milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);  // 100 ms
#else
  usleep(milliseconds * 1000);  // 100 ms
#endif
}

void enumerate_ports() {
  vector<serial::PortInfo> devices_found = serial::list_ports();

  vector<serial::PortInfo>::iterator iter = devices_found.begin();

  while (iter != devices_found.end()) {
    serial::PortInfo device = *iter++;
  }
}

void print_usage() {
  cerr << "Usage: test_serial {-e|<serial port address>} ";
  cerr << "<baudrate> [test string]" << endl;
}

int run() {
  string port("/dev/ttyUSB0");
  enumerate_ports();
  unsigned long baud = 115200;

  serial::Serial my_serial(port, baud, serial::Timeout::simpleTimeout(10));

  /* cout << "Is the serial port open?";
   if (my_serial.isOpen())
     cout << " Yes." << endl;
   else
     cout << " No." << endl;*/

  struct gps_data {
    int date;   /*日期  */
    float time; /* gps定位时间 */
    //
    float heading; /*航向 */
    float pitch;   /*pitch*/
    float roll;    /*roll*/
    //
    double latitude;  /*纬度 */
    double longitude; /* 经度 */
    float altitude;   /*altitude*/
    //
    float speed_v; /* 速度 */
    float speed_n;
    float speed_u;
    //
    float basin_line;
    //
    int NSV1;
    int NSV2;
    //
    char status1;
    //
    char status2;
    //
    char check;
    //
    double UTM_x;
    //
    double UTM_y;
    // UINT check;
  } gps_data;

  // creates a database file 'dbfile.db' if it does not exists.
  database db("dbfile.db");

  // executes the query and creates a 'user' table
  db << "create table if not exists user ("
        "   _id integer primary key autoincrement not null,"
        "   age int,"
        "   name real,"
        "   weight real"
        ");";
  // read data from the serial port
  while (1) {
    string buffer = my_serial.readline(200);

    string firstname(buffer.substr(0, 1));
    string strcompare = "$";
    int compresult = firstname.compare(strcompare);

    if (compresult == 0) {
      const char* bb = buffer.c_str();

      sscanf(bb, "$GPFPD,%d,%f,%f,%f,%f,%lf,%lf,%f,%f,%f,%f,%f,%d,%d,%s%s*",
             &(gps_data.date),        // date
             &(gps_data.time),        // time
             &(gps_data.heading),     // heading
             &(gps_data.pitch),       // pitch
             &(gps_data.roll),        // roll
             &(gps_data.latitude),    // latitude
             &(gps_data.longitude),   // longitude
             &(gps_data.altitude),    // altitude
             &(gps_data.speed_v),     // speed_of_east
             &(gps_data.speed_n),     // speed_of_north
             &(gps_data.speed_u),     // speed_of_sky
             &(gps_data.basin_line),  // basin_line
             &(gps_data.NSV1),        // the_satellite_number_of_first
             &(gps_data.NSV2),        // the_satellite_number_of_second
             &(gps_data.status1),     // GPS_status
             &(gps_data.status2)      // GPS_status
             /*&(gps_data.check)*/);  // CHECK

      UTMalt tm(6378388, 1 / 297.0, 51,
                true);  // International ellipsoid, zone 30n
      {
        // Sample forward calculation
        double lat = gps_data.latitude, lon = gps_data.longitude;  //
        double x, y;
        tm.Forward(lat, lon, x, y);
        gps_data.UTM_x = x;
        gps_data.UTM_y = y;
        // cout << fixed << setprecision(0) << x << " " << y << "\n";
      }

      int date2 = gps_data.date;
      float time2 = gps_data.time;
      float heading2 = gps_data.heading;
      float pitch2 = gps_data.pitch;
      float roll2 = gps_data.roll;

      /*   db << "insert into user "
               "(date,time,heading,pitch,roll) values "
               "(?,?,?,?,?);"
            << date2 << time2 << heading2 << pitch2 << roll2;*/

      // string name = "jack";
      db << u"insert into user (age,name,weight) values (?,?,?);"  // utf16
                                                                   // query
                                                                   // string
         << time2 << heading2 << roll2;

      cout << "buffer=    " << buffer << endl;

      cout << "date:      " << gps_data.date << endl;
      cout << "time:      " << fixed << setprecision(3) << gps_data.time
           << endl;
      cout << "heading:   " << fixed << setprecision(3) << gps_data.heading
           << endl;
      cout << "pitch:     " << fixed << setprecision(3) << gps_data.pitch
           << endl;
      cout << "roll:      " << fixed << setprecision(3) << gps_data.roll
           << endl;
      cout << "latitud:   " << fixed << setprecision(7) << gps_data.latitude
           << endl;
      cout << "longitude: " << fixed << setprecision(7) << gps_data.longitude
           << endl;
      cout << "UTM_x:     " << fixed << setprecision(7) << gps_data.UTM_x
           << endl;
      cout << "UTM_y:     " << fixed << setprecision(7) << gps_data.UTM_y
           << endl;
      cout << "altitude:  " << fixed << setprecision(2) << gps_data.altitude
           << endl;
      cout << "speed_v:   " << fixed << setprecision(3) << gps_data.speed_v
           << endl;
      cout << "speed_u:   " << fixed << setprecision(3) << gps_data.speed_u
           << endl;
      cout << "speed_n:   " << fixed << setprecision(3) << gps_data.speed_n
           << endl;
      cout << "basinLine  " << fixed << setprecision(3) << gps_data.basin_line
           << endl;
      cout << "NSV1:      " << gps_data.NSV1 << endl;
      cout << "NSV2:      " << gps_data.NSV2 << endl;
      switch (gps_data.status2) {
        case '0':
          cout << "Satus:     GPS初始化" << endl;
          break;
        case '1':
          cout << "Satus:     粗对准" << endl;
          break;
        case '2':
          cout << "Satus:     精对准" << endl;
          break;
        case '3':
          cout << "Satus:     GPS定位" << endl;
          break;
        case '4':
          cout << "Satus:     GPS定向" << endl;
          break;
        case '5':
          cout << "Satus:     GPS RTK" << endl;
          break;
        case 'B':
          cout << "Satus:     差分定向" << endl;
          break;
        default:
          cout << "Satus:     状态未知" << endl;
      }

    }
    my_sleep(100);
  }
  return 0;
}

#endif