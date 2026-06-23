import json

with open('data/dialogs/biara_scenes.json', 'r') as f:
    data = json.load(f)

for scene in data:
    if scene['id'] == 'maria':
        scene['id'] = 'dialog_maria_q1_start'
        for choice in scene.get('choices', []):
            if choice['text'] == "Terima Misi":
                choice['next_scene'] = "dialog_maria_q1_accepted"
    elif scene['id'] == 'andreas':
        scene['id'] = 'dialog_andreas_q2_start'
    elif scene['id'] == 'dialog_andreas_q3':
        scene['id'] = 'dialog_andreas_q3_start'
    elif scene['id'] == 'pasar':
        scene['id'] = 'scene_pasar_gandum'
    elif scene['id'] == 'kusir':
        # Was kusir supposed to be something else?
        pass

with open('data/dialogs/biara_scenes.json', 'w') as f:
    json.dump(data, f, indent=2)

