#include "Storage/Storage.cpp"
#include "Register/Register.cpp"
#include "Memory/Memory.cpp"
#include "ALU/ALU.cpp"
#include "CPU/CPU.cpp"
#include "Machine/machine.cpp"

int main() {
    Machine machine;
    machine.displayMenu();
    return 0;
}
