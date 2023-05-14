#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std::string_literals;

namespace Output
{

// Считываем запрос на обработку и вывод из привязанного каталога
void Reader(CatalogueCore::TransporCatalogue& cat);

}