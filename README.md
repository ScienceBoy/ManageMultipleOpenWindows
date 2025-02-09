# ManyOpenWindows
Lost in too many open windows? Cannot find the window you are looking for? Want to automatically arrange 8 Chrome windows, 3 PowerPoint windows and 5 Explorer windows on your second screen?
Here is a powerful tool to control and organize all your open windows with ease. **Minimize**, **Maximize**, **restore**, **arrange** and **Move** one, many or all **windows at once** to any of your screens.

In today's multitasking world, managing multiple open windows efficiently is crucial for productivity. Whether you're a developer, a designer, or just someone who loves to keep things organized, having a tool that helps you manage your open windows can save you time and reduce frustration. The functions provided in "Many Open Windows", like minimize all windows at once or arrange them automatically, were missing for me and my colleagues in Windows OS for many years. 

## Features
- **List All Open Windows**: Displays a list of all currently open windows, including their process names and titles.
- **Minimize, Maximize, Restore, Close, Arrange or Move Open Windows**: Easily minimize, maximize, restore, close, arrange or move selected windows with a single click.

## Installation
Download all files into a folder and start **ManyOpenWindows.exe** from there.

If you prefer, you could also re-build the executables:
1. Clone the repository:
    ```sh
    git clone https://github.com/ScienceBoy/ManyOpenWindows.git
    ```
2. Navigate to the project directory:
    ```sh
    cd ManyOpenWindows
    ```
3. Build the project using your preferred C++ compiler. Example with g++:
    ```sh
    windres resource.rc -o resource.o
    ```
    ```sh
    g++ -std=c++17 ManyOpenWindows.cpp resource.o -o ManyOpenWindows -lgdi32 -luser32 -lcomctl32 -lpsapi -ldwmapi -lpthread -static-libgcc -static-libstdc++ -static -O3 -s -DNDEBUG -mwindowsG
    ```

## Usage
1. Run the executable file generated after building the project.
2. The main window will display a list of all open applications and their windows.
3. Expand or collapse the windows of the applications by clicking the ">" button next to each process name.
4. Use the menues to minimize, maximize, restore, close, arrange or move selected windows.

## Contributing
Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or submit a pull request.

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Privacy Policy
This privacy policy describes how your personal data is collected, used, and protected when you use the "ManyOpenWindows" program.
1. Data Collection and Use
"ManyOpenWindows" does not collect any personal data from its users. The program does not store any information that could identify you, such as names, email addresses, or IP addresses.
2. Data Storage
Since "ManyOpenWindows" does not collect personal data, no data is stored, retained or transferred.
3. Data Sharing
As no personal data is collected, there is no data sharing with third parties.
4. Security
We implement appropriate technical and organizational measures to ensure the security of the program and to prevent unauthorized access to any data.
5. Changes to the Privacy Policy
This privacy policy may be updated from time to time. We recommend that you review this page regularly to stay informed about any changes.
6. Contact
If you have any questions or concerns regarding this privacy policy, please contact us through the contact information provided on our GitHub page.

## Acknowledgements
Special thanks to everyone who contributed to the development and testing of this program.
