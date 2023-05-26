#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"

namespace json
{

// Считываем запрос на добавлениеб обрабатываем в привязанный каталог, сохраняем отдельную информацию в удобные для взаимодействия хранилища
void Reader(Handler::InfoKeeper& keeper, TransporCatalogueLib::CatalogueCore::TransporCatalogue& cat, svg::map_render::MapRender& map);

}