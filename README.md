ESP32 Pet Feeder with Web Interface
===================================

This project implements a pet feeder using an ESP32 microcontroller, enabling users to schedule feeding times and manually trigger feeding sessions through a web interface accessible via a browser.

Features
--------

*   **Web Interface**: Access the pet feeder control panel through a web browser.
*   **Scheduled Feeding**: Program feeding times for your pet.
*   **Manual Feeding**: Initiate feeding sessions at any time via the web interface.
*   **ESP32**: Utilizes the ESP32 microcontroller for internet connectivity and web server capabilities.

Components
----------

*   ESP32 Development Board
*   Servo Motor
*   Power Supply
*   Pet Feeder Mechanism
*   BusBoard and Connectors

Setup
-----

1.  **Hardware Setup**: Connect the servo motor to the ESP32 in GPIO 5.
    
2.  **Software Setup**:
    
    *   Clone or download the code repository to your local machine.
    *   Open the project in your preferred Integrated Development Environment (IDE).
    *   Configure your ESP32 board and upload the code to the microcontroller.
3.  **Connect to Wi-Fi**: Configure your ESP32 to connect to your Wi-Fi network. Update the Wi-Fi credentials in the code if necessary.


Usage
-----

1.  **Setting Feeding Schedule**:
    
    *   Access the web interface.
    *   Navigate to the schedule settings section.
    *   Input the desired feeding times and save the schedule.
2.  **Manual Feeding**:
    
    *   From the web interface, navigate to the manual feeding section.
    *   Trigger a feeding session by clicking the "Feed Now" button.
3.  **Monitoring**:
    
    *   Monitor the feeding activities through the web interface.
    *   Check the logs for feeding history and any errors.
