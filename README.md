# Introduction
This educational project has been designed for learning multiple C++'s concepts of architecting large projects.

## 🪬 Requirements
1. CMAKE 3.11 (or above)
2. g++ compiler 13 (or above)
3. C++ 17 (or above)

## 🔎 Project Overview
The Transport Catalogue class first creates a database from a JSON input containing bus stops (names, distances between them, longitute and latitude), routes (names, stops along the way, total time, etc.), as well as render settings for an SVG map. It then outputs responses for data requests provided in the JSON input data.

![image](https://github.com/AdrianGuretto/transport_navigator_simulator/assets/102734242/6d204b00-1b11-4bdd-b50e-4a7601025f12)
*An example of a rendered transport map*

## 📥 Installation (Linux and Windows)
1. Clone the repository on your system and enter the project folder:
   ```
   git clone https://github.com/AdrianGuretto/transport_navigator_simulator.git
   cd transport_navigator_simulator
   ```
   Alternatively, you can download the code archive from the repository.
2. Create a folder for building the project and build it:
   ```
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```
3. After the build is successful, move the executable file `TransportCatalogue` to a desired folder.

## 📚 Input JSON data format
```
{
  "base_requests": [],
  "render_settings": {},
  "routing_settings": {},
  "stat_requests": [], 
}
```
#### 1. base_requests
This JSON array supports following formats of supplied data inside:
1. Stop:
   ```
   {
     "type": "Stop",
     "name": "stop_name",
     "latitude": 43.412291,
     "longitutde": 41.322910,
     "road_distances": {
       "to_stop1": 2314,
       "to_stop2": 1000,
       "to_stop3": 3000
     }
   }
   ```
2. Bus:
   ```
   {
     "type": "Bus",
     "name": "route_name",
     "stops": [
       "stop1",
       "stop2",
       "stop3"
     ],
     "is_roundtrip": <true/false>
   }
   ```
   *is_roundtrip* indicates whether the bus route is following a circle route.
#### 2. render_settings
```
{
    "bus_label_font_size": 20,
    "bus_label_offset": [
        7,
        15
    ],
    "color_palette": [
        "green",
        [
            255,
            160,
            0
        ],
        "red"
    ],
    "height": 200,
    "line_width": 14,
    "padding": 30,
    "stop_label_font_size": 20,
    "stop_label_offset": [
        7,
        -3
    ],
    "stop_radius": 5,
    "underlayer_color": [
        255,
        255,
        255,
        0.85
    ],
    "underlayer_width": 3,
    "width": 200
}
```
#### 3. routing_settings
```
{
    "bus_velocity": 30,
    "bus_wait_time": 2
}
```
**bus_wait_time** — time, in minutes, needed for a bus to arrive to a stop. We don't take into account other factors such as random events on the road, traffic, and etc. We assume that the waiting time is constant for each bus.
#### 4. stat_requests
`stat_requests` array accepts the following types:
1. Bus
   ```
   {
     "id": 0,
     "type": "Bus",
     "name": "route_name"
   }
   ```
   *Response*:
   ```
   {
    "curvature": 2.18604,
    "request_id": 0,
    "route_length": 9300,
    "stop_count": 4,
    "unique_stop_count": 3
   } 
   ```
  **curvature** — curvature degree of a route (factual distance / geographical distance)
2. Stop
   ```
    {
      "id": 12345,
      "type": "Stop",
      "name": "stop_name"
    }
   ```
   *Response*:
   ```
    {
      "buses": [
          "route_name1", "route_name2"
      ],
      "request_id": 12345
    }
   ```
3. Map
   ```
   {
    "type": "Map",
    "id": 11111
   } 
   ```
   *Response*:
   ```
   {
     "map": <SVG code>,
     "id": 11111
   }
   ```
4. Route
   ```
   {
      "from": "stop1",
      "id": 9,
      "to": "stop2",
      "type": "Route"
   }
   ```
   *Response*:
   ```
   {
      "items": [
          {
              "stop_name": "stop1",
              "time": 2,
              "type": "Wait"
          },
          {
              "bus": "route_name1",
              "span_count": 2,
              "time": 3,
              "type": "Bus"
          },
          {
              "stop_name": "some_stop2",
              "time": 2,
              "type": "Wait"
          },
          {
              "bus": "route_name2",
              "span_count": 1,
              "time": 0.42,
              "type": "Bus"
          }
      ],
      "request_id": 9,
      "total_time": 7.42
   },
   ```
   *Note*: RouteResponse essentially provides a list of objects used for this built route.


