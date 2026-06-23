import json

with open('data/dialogs/biara_scenes.json', 'r') as f:
    data = json.load(f)

for scene in data:
    if scene['id'] == 'dalam_menara':
        scene['id'] = 'scene_lukas_found'
    elif scene['id'] == 'serangan_bandit':
        scene['id'] = 'scene_serangan_bandit'
        # Add battle for Q3
        scene['on_exit'] = ["start_battle bandit_scum,bandit_scum,bandit_scum"]
    elif scene['id'] == 'scene_pasar_gandum':
        # Ensure it has the right on_exit battle
        scene['on_exit'] = ["start_battle bandit_scum,bandit_scum \"give_item bread 1\""]

with open('data/dialogs/biara_scenes.json', 'w') as f:
    json.dump(data, f, indent=2)

