#pragma once

#include "../Map_renderer/map_renderer.h"
#include "../Request_handler/request_handler.h"
#include "../Router/transport_router.h"
#include "../Serialization/serialization.h"
#include "../Transport_Catalogue/transport_catalogue.h"
#include "json.h"

namespace TransporCatalogueLib {

namespace jsonOperator {

// Считываем запрос на добавлениеб обрабатываем в привязанный каталог, сохраняем
// отдельную информацию в удобные для взаимодействия хранилища
void Reader(Serialization::SerializationClass &serialize,
            Handler::InfoKeeper &keeper, const bool serialization);

} // namespace jsonOperator

} // namespace TransporCatalogueLib
