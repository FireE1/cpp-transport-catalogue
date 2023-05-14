#pragma once

#include "transport_catalogue.h"

namespace TransporCatalogueLib
{

namespace Input
{

// Считываем запрос на добавление и обрабатываем в привязанный каталог
void Reader(CatalogueCore::TransporCatalogue& cat);

}

}