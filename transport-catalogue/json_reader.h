#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace TransporCatalogueLib
{

namespace jsonOperator
{

// Считываем запрос на добавлениеб обрабатываем в привязанный каталог, сохраняем отдельную информацию в удобные для взаимодействия хранилища
void Reader(Handler::InfoKeeper& keeper, CatalogueCore::TransporCatalogue& cat, map_render::MapRender& map);

}

}