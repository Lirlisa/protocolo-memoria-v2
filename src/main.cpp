#include <memory_allocator/memory_allocator.hpp>
#include <memory_allocator/memory_handler/memory_handler.hpp>
#include <Arduino.h>


void setup() {
    Serial.begin(9600);
    while (!Serial) {};
    Serial.println("Serial begin");
    Memory_allocator<12> memoria;
    Serial.println(1);
    Memory_handler& handler1 = memoria.acquire<int>(9);
    Memory_handler& handler2 = memoria.acquire<int>(10);
    Memory_handler& handler3 = memoria.acquire<int>(11);
    memoria.release(handler2);
    Memory_handler& handler4 = memoria.acquire<int>(12);
    Serial.println(handler1.es_valido() ? "True" : "False");
    Serial.println(handler2.es_valido() ? "True" : "False");
    Serial.println(handler3.es_valido() ? "True" : "False");
    Serial.println(handler4.es_valido() ? "True" : "False");
    int& num1 = handler1.get_elem<int>();
    int& num3 = handler3.get_elem<int>();
    int& num4 = handler4.get_elem<int>();
    Serial.println(num1);
    Serial.println(num3);
    Serial.println(num4);
}

void loop() {
    // put your main code here, to run repeatedly:

}
