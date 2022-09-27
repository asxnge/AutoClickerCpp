#include <iostream>
#include <string>
#include <Windows.h>
#include <thread>
#include <mutex>
#include <fstream>

std::mutex mutex;
int cps = 10;
int mouse_button[2][2] = {{MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP}, {MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP}};

bool set_block = false;         //нажимать пкм
bool toggle_button = false;     //зажать режим клика
bool set_toggle = false;        //работает режим toggle
bool set_bridge = false;        //работает макрос fast bridge

void bridge(){      //макрос fast bridge
    while(true){
        mutex.lock();
        if (set_bridge && GetAsyncKeyState(VK_MBUTTON)){     //средняя клавиша мыши
            mutex.unlock();
            keybd_event(VK_LSHIFT,0,0,0);                       //левый шифт
            std::this_thread::sleep_for(std::chrono::milliseconds(91));
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);    //правая клавиша мыши
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);      //правая клавиша мыши
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            keybd_event(VK_LSHIFT,0,KEYEVENTF_KEYUP,0);                //левый шифт
            std::this_thread::sleep_for(std::chrono::milliseconds(199));
        }  else {
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void mouse_click(int _mouse_button[]){                   //нажатие мыши
    mouse_event(_mouse_button[0], 0, 0, 0, 0);  //нажимает клавишу мыши
    mouse_event(_mouse_button[1], 0, 0, 0, 0);  //отпускает клавишу мыши
}

void click(bool* priority){
    if(!set_block || set_block && !(*priority)) mouse_click(mouse_button[0]);  //левая клавиша мыши
    else if(set_block && (*priority)) mouse_click(mouse_button[1]);            //правая клавиша мыши
    *priority = !(*priority);
}

void mouse(){
    bool priority = false;     //очередность
    while(true){
        mutex.lock();
        if (toggle_button && set_toggle || !set_toggle && GetAsyncKeyState(VK_INSERT)){
            mutex.unlock();
            click(&priority);
            mutex.lock();
            int time = 1000 / cps;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(time));
        } else {
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void toggle_mode(){
    while(true){
        if (set_toggle && GetAsyncKeyState(VK_INSERT)){       //клавиша Ins
            mutex.lock();
            toggle_button = !toggle_button;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        } else std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void save(std::string link){
    link.erase(link.begin(),link.begin() + link.rfind('\\') + 1);
    std::ofstream file(link + ".bin", std::ios::binary);
    mutex.lock();
    file.write((char*)&cps,sizeof(cps));
    file.write((char*)&set_toggle,sizeof(set_toggle));
    file.write((char*)&set_block, sizeof(set_block));
    file.write((char*)&set_bridge,sizeof(set_bridge));
    file.write((char*)mouse_button,sizeof(mouse_button));
    mutex.unlock();
    file.close();
}

void load(std::string link){
    link.erase(link.begin(),link.begin() + link.rfind('\\') + 1);
    std::ifstream file(link + ".bin", std::ios::binary);
    if(!file.is_open()) return;
    mutex.lock();
    file.read((char*)&cps,sizeof(cps));
    file.read((char*)&set_toggle,sizeof(set_toggle));
    file.read((char*)&set_block, sizeof(set_block));
    file.read((char*)&set_bridge,sizeof(set_bridge));
    file.read((char*)mouse_button,sizeof(mouse_button));
    mutex.unlock();
    file.close();
}

bool correct_number(std::string number){
    if (number.find_first_not_of("0123456789") != std::string::npos) return false;
    return (std::stoi(number) > 0 && std::stoi(number) <= 100);
}

int main(int argc, char* argv[]){
    SetConsoleTitle("AutoClicker by MrsEmilia");
    system("color A");
    load(*argv);
    std::cout << "Cps: " << cps << "\n";
    std::cout << "Toggle: " << (set_toggle ? "on" : "off") << "\n";
    std::cout << "Block: " << (set_block ? "on" : "off") << "\n";
    std::cout << "Bridge: " << (set_bridge ? "on" : "off") << "\n";
    std::cout << "Main button: " << (mouse_button[0][0] == MOUSEEVENTF_LEFTDOWN ? "left" : "right") << "\n";
    std::thread m(mouse);
    std::thread b(bridge);
    std::thread t(toggle_mode);
    m.detach();
    b.detach();
    t.detach();
    std::cout << "Start AutoClicker\n";
    std::cout << "Commands: cps, block, toggle, bridge, button, stop\n";
    while (true){
        std::string command;
        std::cin >> command;
        if(command == "cps") {
            do{
                std::cout << "Input cps: ";
                std::cin >> command;
            } while (!correct_number(command));
            mutex.lock();
            cps = std::stoi(command);
            mutex.unlock();
            std::cout << "cps: " << command << "\n";
            save(*argv);

        } else if(command == "block"){
            mutex.lock();
            set_block = !set_block;
            std::cout << "Block set: " << (set_block ? "on" : "off") << "\n";
            mutex.unlock();
            save(*argv);

        } else if(command == "toggle"){
            mutex.lock();
            toggle_button = false;
            set_toggle = !set_toggle;
            std::cout << "Toggle set: " << (set_toggle ? "on" : "off") << "\n";
            mutex.unlock();
            save(*argv);

        } else if(command == "bridge"){
            mutex.lock();
            set_bridge = !set_bridge;
            std::cout << "Bridge set: " << (set_bridge ? "on" : "off") << "\n";
            mutex.unlock();
            save(*argv);

        } else if(command == "button"){
            mutex.lock();
            std::swap(mouse_button[0], mouse_button[1]);
            std::cout << "Main button: " << (mouse_button[0][0] == MOUSEEVENTF_LEFTDOWN ? "left" : "right") << "\n";
            mutex.unlock();
            save(*argv);

        } else if(command == "stop"){
            break;
        } else std::cout << "Error!\n";
    }
}
