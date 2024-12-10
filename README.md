Smart Home Automation System

This project demonstrates an advanced smart home automation system built with Arduino, IoT, and Python technologies. It integrates multiple features to enhance home security, convenience, and environmental monitoring.

Features:
Motion Sensor at Main Gate:
Detects movement and automatically turns on the light.

Smart Password Door Lock System:
Includes an ultrasonic sensor, number keypad, and LCD display.
Displays "Welcome" and prompts for a password upon detecting presence.
Allows a maximum of 3 attempts for correct password entry.
Failsafe actions include sounding an alarm, sending notifications, and updating a real-time Google Sheet database.
Unlocks the door for 10 seconds upon correct password entry, then locks automatically.

Fire Alarm System:
Monitors gas levels, temperature, humidity, and light intensity using sensors.
Activates alarms, sends notifications, and logs data when safety thresholds are exceeded.

Real-Time Monitoring:
A Python-based application displays live data for temperature, humidity, gas, and light levels.

Voice-Controlled Wall Lights:
Controls 4 wall lights using voice commands via a Python-based application.
Commands include "Turn on" and "Turn off."

Room Lights via Blynk App:
Controls 2 room lights using the Blynk app for remote operation.

Real-Time Database Integration:
Logs all actions and sensor updates automatically in a Google Sheet.

Technologies Used:
Hardware: Arduino, Ultrasonic Sensor, Gas Sensor, LDR, Temperature and Humidity Sensors, LCD Display, Keypad.
Software: Python, Blynk IoT Platform, Google Sheets API.
