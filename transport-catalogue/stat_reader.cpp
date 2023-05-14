#include "stat_reader.h"

#include <iostream>
#include <iomanip>
#include <string>

namespace TransporCatalogueLib
{

namespace Output
{

// Ввыводим информацию о маршруте(автобусе) и печатем ее
static void PrintBus(std::string_view bus_name, const CatalogueCore::TransporCatalogue& cat) {
    // Получаем ссылку на автобус(маршрут) по заданному имени
    const CatalogueCore::Bus* bus = cat.FindBus(bus_name);
    // Если автобус(маршрут) не найден
    if (bus == nullptr)
    {
        // Печатаем об отсутствии автобуса(маршрута)
        std::cout << "Bus " << bus_name << ": not found" << std::endl;
    }
    // Если автобус найден
    else
    {
        // Сохраняем всю нужную информацию для вывода и печатаем ее
        auto info = cat.GetBusInfo(bus);
        std::cout << "Bus " << bus->bus << ": " << bus->stops_for_bus_.size() <<
        " stops on route, " << info.unique_stops_on_bus << " unique stops, " <<
        info.route_distance << " route length, " << std::setprecision(6) << 
        info.route_distance / info.geo_distance_between_stops << " curvature" << std::endl;
    }
}

// Ввыводим информацию о остановке и печатаем ее
static void PrintStop(std::string_view stop_name, const CatalogueCore::TransporCatalogue& cat) {
    // Если остановка не найдена
    if (!cat.FindStop(stop_name))
    {
        // Печатаем об отсутствии остановки
        std::cout << "Stop " << stop_name << ": not found" << std::endl;
    }
    // Если остановка найдена
    else
    {
        // Сохраняем маршруты(автобусы), проходящие через остановку
        const auto& buses = cat.GetRoutsOnStop(stop_name);
        // Если таких не имеится
        if (buses.empty())
        {
            // Печатаем об отсутствии маршрутов
            std::cout << "Stop " << stop_name << ": no buses" << std::endl;
        }
        // Если такие имеются
        else
        {
            // Печатаем информацию о остановке
            std::cout << "Stop " << stop_name << ": buses";
            for (const auto& bus : buses)
            {
                std::cout << " " << bus;
            }
            std::cout << std::endl;
        }
    }
}

// Считываем запрос на обработку и вывод из привязанного каталога
void Reader(const CatalogueCore::TransporCatalogue& cat) {
    // string объект для количества вводимых команд
    std::string number_of_commands_str;
    // Получаем первую строчку - количество вводимых команд
    std::getline(std::move(std::cin), number_of_commands_str);
    // Переводим string объект для количества вводимых команд в int объект
    int rotations = std::stoi(number_of_commands_str);
    // Итерируем вводимые команды заданное количество раз
    for (int reads = 0 ; reads < rotations; ++reads)
    {
        // string объект для строчки вводимой команды
        std::string queue;
        // Получаем строку с командой
        std::getline(std::move(std::cin), queue);
        // Ищем первый пробел в команде
        const auto first_space = queue.find_first_of(' ');
        // Создаем string объект для типа вводимой команды
        std::string command = queue.substr(0, first_space);
        // Отрезаем от строки команды ее тип
        queue = queue.substr(first_space + 1, queue.size());
        // Если тип команды Stop
        if (command == "Stop")
        {
            // Ввыводим информацию о остановке и печатаем ее
            PrintStop(queue, cat);
        }
        // Если тип команды Bus
        else if (command == "Bus")
        {
            // Ввыводим информацию о маршруте(автобусе) и печатем ее
            PrintBus(queue, cat);
        }
        // При неккоректной команде пропускаем шаг
        else
        {
            continue;
        }
    }
}

}

}