// Bloqueos de funciones que no existian en la 1.0.
//
// Este archivo esta separado a proposito: si al compilar el error apunta a una
// de estas funciones ("no member named ..."), borra esa funcion (o todo el
// archivo) y el resto del mod sigue funcionando. [VERIFICAR] nombres de metodos.

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class $modify(Editor10Blocks, EditorUI) {
    // Sin copiar / pegar / duplicar
    void onCopy(CCObject*) {}
    void onPaste(CCObject*) {}
    void onDuplicate(CCObject*) {}
    void onCopyState(CCObject*) {}
    void onPasteState(CCObject*) {}

    // Sin modo libre ni rotacion libre
    void toggleFreeMove(CCObject*) {}
    void toggleEnableRotate(CCObject*) {}
};
