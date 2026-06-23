import json

with open('data/dialogs/biara_scenes.json', 'r') as f:
    data = json.load(f)

for scene in data:
    if scene['id'] == 'dialog_maria_q1_start':
        scene['choices'] = [
            {
                "text": "Terima Misi",
                "next_scene": "dialog_maria_q1_accepted",
                "on_exit": ["accept_quest quest_gereja_1"]
            },
            {
                "text": "Tolak Misi",
                "next_scene": "biara_01"
            }
        ]

with open('data/dialogs/biara_scenes.json', 'w') as f:
    json.dump(data, f, indent=2)

