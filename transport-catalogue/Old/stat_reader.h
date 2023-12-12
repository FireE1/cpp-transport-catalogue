#pragma once

#include "transport_catalogue.h"

namespace TransporCatalogueLib
{

namespace Output
{

// Считываем запрос на обработку и вывод из привязанного каталога
void Reader(const CatalogueCore::TransporCatalogue& cat);

}

}