Monitoring Radon Levels with Arduino
===========================================

* Arduino (Uno) program to interface Safety Siren Pro Series 3 Radon Gas Detector Model No: HS71512
* Periodically reads radon level from 7-segment indicators and publish to Xively service to display graphs.
* Note: Hs71512 requires hardware modification to connect to Arduino.
* Requires Xively (https://github.com/xively/xively_arduino) and HttpClient (https://github.com/amcewen/HttpClient/releases) Arduino libraries.
Before using add your Xively API key (see line in code below):
```
    char xivelyKey[] = "put your key here";
```

Based on works of Chris Nafis (http://www.howmuchsnow.com) for Safety Siren Radon Gas Detector Model No: HS80002

Modified by Andriy Rokhmanov (http://rokhmanov.blogspot.com) for HS71512


