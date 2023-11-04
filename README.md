# Waste bin reminder

![waste bin reminder](png/waste_bin_600_v.png "waste bin reminder")

## Some infos

I liked the idea of an illuminated waste-bin to remind us to put out the rubbish bins. Meanwhile we have 5 different bins, so the task is not as simple as it was years ago :)

### BOM

- Lolin C3 mini
- 3D printed parts

### Software

The software communicates per `MQTT` over `WiFi`. It uses my ESPToolbox library for WiFi and logging functions (UDP) and to get the time. To avoid version problems the lib is included in the sketch folder. Programming is done over OTA.

All relevant data you must or can change are in a config (`config.h`) file, also contained in the main folder. You can also place this file in the sketchbook/library folder named `Secrets` and rename it to your needs (e.g. `secrets.h`).

In the main file (.ino) you can activate or deactivate features by commenting or uncommenting some lines. Here you must activate `USE_SECRETS`:

In the `config.h` file, you must change the WiFi parameter and the MQTT server address. In the automated watering array we define the start times and duration of watering events.

Other things we perhaps want to change are the publishing interval `PUBLISH_TIME` or the MQTT topics.

## All infos on: <http://www.weigu.lu/microcontroller/waste_bin_reminder/index.html>
