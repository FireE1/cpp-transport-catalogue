// #include "transport_catalogue.h"
#include "transport_catalogue.cpp"

#include "input_reader.h"
#include "input_reader.cpp"
#include "stat_reader.h"
#include "stat_reader.cpp"

int main() {
    // Создаем каталог
    CatalogueCore::TransporCatalogue cat;
    // Производим ввод команд для записи
    Input::Reader(cat);
    // Производим ввод команд для вывода
    Output::Reader(cat);
}