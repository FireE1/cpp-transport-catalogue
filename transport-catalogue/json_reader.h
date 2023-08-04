#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_router.h"
#include "serialization.h"

namespace TransporCatalogueLib
{

namespace jsonOperator
{

// Считываем запрос на добавлениеб обрабатываем в привязанный каталог, сохраняем отдельную информацию в удобные для взаимодействия хранилища
void Reader(Serialization::SerializationClass& serialize, Handler::InfoKeeper& keeper, const bool serialization);

}

}