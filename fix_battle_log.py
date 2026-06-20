import re

with open("src/states/BattleState.cpp", "r") as f:
    content = f.read()

new_content = re.sub(r'battle_log\s*=\s*(.+?);', r'add_log(\1);', content)

# Also append the add_log definition to the end
new_content += """
void BattleState::add_log(const std::string& msg) {
    battle_log.push_back(msg);
    if (battle_log.size() > 50) {
        battle_log.erase(battle_log.begin());
    }
}
"""

with open("src/states/BattleState.cpp", "w") as f:
    f.write(new_content)
