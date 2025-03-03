![Repository Banner](docs/graphic_materials/github_repo_animation.gif)
# 42 Smart Cluster Sign
## Table of Contents

- [Introduction](#introduction)
- [Usage](#usage)
- [Features](#features)
- [Components](#components)
- [Contributing](#contributing)
- [Suggestions for Contributions](#suggestions-for-contributions)
- [Regards](#regards)
- [License](#license)
- [Conclusion](#conclusion)


## Introduction

Welcome to the README for the 42 Smart Cluster Sign — a self-sufficient information display designed to be installed on a cluster door. Its purpose is to notify students when the cluster is reserved for an exam and prevent them from accidental entering. The device automatically retrieves exam dates from Intra and displays appropriate warnings and information on its e-paper screen. In the spare time it simply displays the cluster number.

For more information, please, refer to the technical documentation in the docs folder of this repository.


## Usage

Once the 42 Smart Cluster Sign is installed and powered on, it will automatically retrieve exam dates from Intra and update the display accordingly. The device operates in the following states:
1. **Normal state**: The device displays the number of the cluster, accompanied by pictograms that depict some of the cluster use cases: cooperating with fellow peers, working on a computer, and studying from books. The last pictogram indicates "free to enter" contrasting with the "do not enter" cross pictogram that appears during exams. In case the Sign needs to communicate an issue with its operation, the pictograms are replaced with an appropriate error message, e.g.: "Low battery" or "Could not get exam info".
2. **Pre-exam warning**: Early in the morning of an exam day, the device hides the pictograms and adds a small informational note about the upcoming exam later in the day. This way, students who want to settle into the room can plan their work ahead with the cluster availability limitations in mind.
3. **1-hour countdown**: 1 hour before the exam, a countdown begins. The device no longer displays the cluster number but a full-screen sign with the minutes left until the exam, indicating to students that it is time to vacate the classroom.
4. **Exam in progress**: During the exam, the device displays a big bright-red warning sign indicating the ongoing exam. The drastically changed colour palette attracts attention, enhancing the sign's effectiveness.
5. **Post-exam state**: After the exam ends, the Sign returns to its normal state, displaying the number of the cluster.


## Features

- Highly autonomous operation: the Sign operates fully autonomously and does not require any input from the school personnel;
- Automatic exam time check: the Sign keeps track of the exams published on Intra on its own;
- Adaptation to changes: the Sign can adapt its behaviour to some unusual exam scheduling, such as last-minute cancellations, varying exam lengths, or sequences of several immediate exams;
- Exam subscribers check: if there is noone undertaking the exam, the Sigh will know it and will not block the room;
- Automatic battery charge monitoring: you will surely know when to charge the device;
- Deep Sleep mode: the microcontroller sleeps most of the time and wakes up only when it has a task to do, drastically saving the battery charge;
- Telegram enabled: control the Sign, update its security token, and receive operational information — all remotely, via your Telegram chat;
- Over The Air firmware update: develop and upload new features without the need of looking for a cable;
- Watchdog: no program failure can stop the Sign as its inner watchdog makes sure the program execution does not get stuck;
- Low-maintenance design: the 4000mAh battery ensures long periods of operation - up to a few months - between recharges, which require a standard USB-C cable and any 5V power adapter. During recharging, the Sign remains fully operational.


## Components

The following components are used in the 42 Smart Cluster Sign:
1. **Seeed Studio XIAO ESP32C3 Wi-Fi module**: provides connectivity to the Internet for retrieving exam dates, updates the display, takes care of the battery with its battery management and charging circuit — all of that while rocking modern USB-C port for charging and easy software updating.
2. **Good Display GDEY075Z08 7.5" 800x480 ePaper black/red/white SPI display**: a high-resolution display that allows for clear and easy-to-read information.
3. **Good Display DESPI-C02 universal SPI e-Paper adapter**: transforms display's FPC interface into microcontroller's SPI.
4. **4000mAh Li-ion battery with overcharge and undercharge protection**: powers the device and ensures continuous operation.
5. **IKEA RÖDALM photo frame, black, 13x18 cm**: made a good enclosure.
6. **Custom 3D-printed board**: to hold all the electronics in place.

For more information, please, refer to the bill of materials in the docs folder of this repository.


## Contributing

Contributions to the 42 Smart Cluster Sign project are welcome! If you have any ideas, bug fixes, or improvements, feel free to open an issue, fork or submit a pull request on the GitHub repository.

When contributing, please adhere to the existing code style and follow the established guidelines. Clearly describe your changes and provide any necessary documentation or tests.


## Suggestions for Contributions

**good first contributions**
- Core temperature monitoring as a safety mechanism: the temperature being too high may signal some serious hardware failure and a safety threat, e.g. the battery has the lowest maximum allowed operational temperature of 60°C — notify the Telegram chat if the temperature of the core gets at least to 50°C;
- Forced display refresh: the Sign is programed not to refresh the content on the display unless it is absolutely necessary, but this approach causes some graphical artifacts to appear with time. Add a scheduled full display cleaning. Once a week is enough. Preferably in the night when noone can see it. After the cleaning process is done, the Sign may not stay blank and shall display relevant information.
- Refactor the project to lower the number of the Common global (com_g) variables. But DO NOT alter the RTC global (rtc_g) variables unless you are absolutely certain of what you are doing;
- Add a countdown till the end of an ongoing exam like the already existing one till the start of an upcoming exam. 

**advanced contributions**
- Adding a "DO NOT ENTER!" sign that can be displayed or errased from the screen by pressing the WARNING_BUTTON button;
- Remove the ubiquitous updating of time and date from the NTP server. Instead, the time and date can be obtained from any Intra server message, similar to how the exam date and time are retrieved. Implement the process of obtaining the time and date from the Intra server messages.
- Core temperature monitoring as a mechanism for detecting the battery charging process being finished: when the battery is being charged, the core temperature of the microcontroller rises approximatelly 10°C above its common operational temperature and then drops back down when finished charging — notify the Telegram chat when charging is finished. This functionality may not block the Sign from fulfilling its main goal of notifying students about exams;
- Logging error messages into a log file in the microcontroller's memory and the log file extraction mechanism;
- The direct OTA implementation - the Sign itself creates a Wi-Fi access point that you can connect to and to update the firmware without the need of a Wi-Fi modem in the middle;
- Replacing Telegram with Slack: refactoring all the Sign's Telegram functionality to work via Slack would allow integrating the Sign better into the 42 ecosystem as well as developing new functionality, e.g. notifying all the students about an upcomming exam.

The **Issues** tab of this repository may also be worth checking, as it might contain suggestions that have not yet made their way onto this list.


## Regards

This project is a group effort of various 42 students with support from the 42 Prague Bocal team. Here they are:
- **raleksan**, 42 Prague — software and electronic hardware developer. Made the idea reality,
- **gsura**, 42 Prague — idea starter and motivation supporter. Pitched the idea to Bocal so well they ended up asking for two devices,
- **cgray**, 42 Prague — hardware development. Modeled beautiful and functional inner frame for the electronics,
- **phelebra**, 42 Prague — 3D printing. Turned 3D model into high-quality pieces of hardware.
- **arosado-**, 42 Lisboa — 42 API expert. Tought how to get the data from the 42 servers,
- **jrathelo**, 42 Nice — memory wizard. Helped resolve the microcontroller's RAM overflow.

The project is based on Jean-Marc Zingg's [GxEPD2](https://github.com/ZinggJM/GxEPD2) library for e-paper displays.
The project uses the [ArduinoOTA](https://github.com/jandrassy/ArduinoOTA) library by Juraj Andrassy for the Over-The-Air software update functionality.
The Sign's Telegram Bot functionality is provided by Brian Lough's [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) library.


## License

The 42 Smart Cluster Sign project is licensed. Please, familiarise yourself with the license before using the software or working on it. The text of the license can be found in this repository.

Please note that while the project strives to provide accurate information, it is provided "as is" without any warranty. Use the project at your own risk.


## Conclusion

Thank you for your interest in the 42 Smart Cluster Sign project! We hope this README provides you with the necessary information to understand the project's purpose, features, installation process, usage, and maintenance. If you did not find the information you need, please, refer to the technical documentation in this repository. If you have any further questions or need assistance, please don't hesitate to reach out. Happy coding!