#include <iostream>
#include "src/db/DB.hpp"

int main() {
    DB db;
    db.load_data();
    auto silas = db.get_npc("npc_silas");
    if (silas) {
        std::cout << "Silas magics count: " << silas->get_magics().size() << "\n";
        for (const auto& m : silas->get_magics()) {
            std::cout << "Magic: " << m.name << "\n";
        }
    }
    return 0;
}
