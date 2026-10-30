// Editor 1.0 - convierte el editor de GD 2.2081 en el de la 1.0.
//
// OJO: escrito sin poder compilar ni probar. Lo que depende de nombres internos
// del juego (IDs de objetos, tags de botones, nombres de miembros) esta marcado
// con [VERIFICAR]. Casi todo se puede ajustar desde los ajustes del mod, sin
// recompilar.

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditButtonBar.hpp>

#include <cstdlib>
#include <set>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

using namespace geode::prelude;

namespace {

// Orden de las capturas de la 1.0. [VERIFICAR] los IDs salen de memoria y de
// las capturas. Faltan pilares, cadena y decoracion (no pude confirmar sus IDs).
// Si un hueco sale mal, usa el ajuste "palette-override".
std::vector<int> const DEFAULT_PALETTE = {
    // pag. 1: bloques, picos, losa, pad amarillo
    1, 2, 3, 4, 5, 6, 7, 8, 39, 9, 40, 35,
    // pag. 2: orbe amarillo y portales (azul, amarillo, cubo, nave)
    36, 10, 11, 12, 13,
    // pag. 4: triggers (efectos de entrada, fondo, suelo, start pos, estela)
    22, 23, 24, 25, 26, 27, 29, 30, 31, 33, 32,
};

constexpr int COLUMNS = 6;
constexpr int ROWS = 2;

std::vector<std::string> splitCsv(std::string const& text) {
    std::vector<std::string> out;
    std::stringstream ss(text);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        auto b = tok.find_first_not_of(" \t\r\n");
        if (b == std::string::npos) continue;
        auto e = tok.find_last_not_of(" \t\r\n");
        out.push_back(tok.substr(b, e - b + 1));
    }
    return out;
}

std::vector<int> parseInts(std::string const& text) {
    std::vector<int> out;
    for (auto const& t : splitCsv(text)) {
        char* end = nullptr;
        long v = std::strtol(t.c_str(), &end, 10);
        if (end != t.c_str() && *end == '\0' && v > 0) {
            out.push_back(static_cast<int>(v));
        }
    }
    return out;
}

std::vector<int> paletteIds() {
    auto custom = parseInts(Mod::get()->getSettingValue<std::string>("palette-override"));
    return custom.empty() ? DEFAULT_PALETTE : custom;
}

std::set<int> blockedEditTags() {
    auto v = parseInts(Mod::get()->getSettingValue<std::string>("blocked-edit-tags"));
    return std::set<int>(v.begin(), v.end());
}

void dumpTree(CCNode* node, int depth, int& budget) {
    if (!node || depth > 3 || budget <= 0) return;
    --budget;
    log::info(
        "{}{} id='{}' tag={} vis={}",
        std::string(depth * 2, ' '), typeid(*node).name(),
        node->getID(), node->getTag(), node->isVisible()
    );
    CCObject* obj = nullptr;
    CCARRAY_FOREACH(node->getChildren(), obj) {
        dumpTree(static_cast<CCNode*>(obj), depth + 1, budget);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Editor: paleta unica (sin pestañas) con solo objetos de la 1.0
// ---------------------------------------------------------------------------
class $modify(Editor10UI, EditorUI) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorUI::init(lel)) return false;

        // Oculta interfaz que no existia en la 1.0 (por ID de Node IDs).
        // [VERIFICAR] los IDs por defecto son suposiciones; los que no existan se ignoran.
        for (auto const& id : splitCsv(Mod::get()->getSettingValue<std::string>("hide-node-ids"))) {
            if (auto node = this->getChildByIDRecursive(id)) {
                node->setVisible(false);
                if (auto parent = node->getParent()) parent->updateLayout();
            }
        }

        if (Mod::get()->getSettingValue<bool>("dump-ui")) {
            int budget = 400;
            log::info("=== Editor 1.0: volcado de EditorUI ===");
            dumpTree(this, 0, budget);
        }
        return true;
    }

    void setupCreateMenu() {
        EditorUI::setupCreateMenu();

        // Todas las barras reciben la misma paleta 1.0; asi da igual que pestaña
        // estuviera guardada como activa. [VERIFICAR] m_createButtonBars / m_tabsMenu.
        auto ids = paletteIds();
        for (auto bar : CCArrayExt<EditButtonBar*>(m_createButtonBars)) {
            auto items = CCArray::create();
            for (int id : ids) {
                if (auto btn = this->getCreateBtn(id, 4)) items->addObject(btn);
            }
            bar->loadFromItems(items, COLUMNS, ROWS, false);
        }

        // La 1.0 no tenia pestañas de categorias.
        if (m_tabsMenu) m_tabsMenu->setVisible(false);
    }
};

// ---------------------------------------------------------------------------
// Pestaña Edit: quita medio bloque, 45°, rotacion libre, etc.
// ---------------------------------------------------------------------------
class $modify(Editor10Bar, EditButtonBar) {
    void loadFromItems(CCArray* items, int columns, int rows, bool keepPage) {
        // Solo barras que NO son de construccion (las de construccion usan CreateMenuItem).
        if (items && items->count() > 0 && !typeinfo_cast<CreateMenuItem*>(items->objectAtIndex(0))) {
            auto blocked = blockedEditTags();
            auto kept = CCArray::create();
            std::string tags;
            bool removed = false;

            CCObject* obj = nullptr;
            CCARRAY_FOREACH(items, obj) {
                if (auto item = typeinfo_cast<CCMenuItem*>(obj)) {
                    tags += std::to_string(item->getTag()) + " ";
                    // [VERIFICAR] se asume que el tag del boton es el comando de edicion.
                    if (blocked.count(item->getTag())) {
                        removed = true;
                        continue;
                    }
                }
                kept->addObject(obj);
            }

            if (Mod::get()->getSettingValue<bool>("dump-ui")) {
                log::info("EditButtonBar (no construccion), tags en orden: {}", tags);
            }
            if (removed) items = kept;
        }
        EditButtonBar::loadFromItems(items, columns, rows, keepPage);
    }
};
