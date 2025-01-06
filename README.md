# ManageMultipleOpenWindows
Lost in too many open windows? Cannot find the window you are looking for?
Here is a powerful way to control and organize all your open windows with ease. **Minimize**, **restore** and **arrange** one, many or all **windows at once**

In today's multitasking world, managing multiple open windows efficiently is crucial for productivity. Whether you're a developer, a designer, or just someone who loves to keep things organized, having a tool that helps you manage your open windows can save you time and reduce frustration. The functions provided in "Manage Multiple Open Windows", like minimize all windows at once or arrange them automatically, were missing for me and my colleagues in Windows OS. 

## Features
- **List All Open Windows**: Displays a list of all currently open windows, including their process names and titles.
- **Minimize, Restore, Close, and Arrange Windows**: Easily minimize, restore, close, or arrange selected windows with a single click.

## Installation
Download all files into a folder and start **ManageMultipleOpenWindows.exe** from there.

If you prefer, you could also re-build the executables:
1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/manage-multiple-open-windows.git
    ```
2. Navigate to the project directory:
    ```sh
    cd manage-multiple-open-windows
    ```
3. Build the project using your preferred C++ compiler. Example with g++:
    ```sh
    windres resource.rc -o resource.o
    ```
    ```sh
    g++ ManageMultipleOpenWindows.cpp resource.o -o ManageMultipleOpenWindows -lgdi32 -luser32 -lpsapi -lcomctl32 -static-libgcc -static-libstdc++ -static -mwindows -lpthread
    ```

## Usage
1. Run the executable file generated after building the project.
2. The main window will display a list of all open applications and their windows.
3. Expand or collapse the windows of the applications by clicking the ">" button next to each process name.
4. Use the buttons to minimize, restore, close, or arrange selected windows.

## Contributing
Contributions are welcome! If you have any ideas, suggestions, or bug reports, please open an issue or submit a pull request.

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgements
Special thanks to everyone who contributed to the development and testing of this program.
