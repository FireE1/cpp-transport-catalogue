#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <map>
#include <utility>

using namespace std::string_literals;

namespace Input
{

// Считываем запрос на добавление и обрабатываем в привязанный каталог
void Reader(CatalogueCore::TransporCatalogue& cat);

}