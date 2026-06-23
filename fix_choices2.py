import json

with open('data/dialogs/biara_scenes.json', 'r') as f:
    data = json.load(f)

for scene in data:
    if scene['id'] == 'dialog_andreas_q2_start':
        scene['choices'] = [
            {
                "text": "Terima Misi",
                "next_scene": "asrama",
                "on_exit": ["accept_quest quest_gereja_2"]
            },
            {
                "text": "Tolak Misi",
                "next_scene": "biara_02"
            }
        ]
    elif scene['id'] == 'dialog_andreas_q3_start':
        scene['choices'] = [
            {
                "text": "Terima Misi",
                "next_scene": "gerbang_biara",
                "on_exit": ["accept_quest quest_gereja_3"]
            },
            {
                "text": "Tolak Misi",
                "next_scene": "biara_03"
            }
        ]

with open('data/dialogs/biara_scenes.json', 'w') as f:
    json.dump(data, f, indent=2)

