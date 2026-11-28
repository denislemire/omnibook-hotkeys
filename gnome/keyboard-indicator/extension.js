import Atk from 'gi://Atk';
import GObject from 'gi://GObject';
import GIRepository from 'gi://GIRepository?version=3.0';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';
import * as PanelMenu from 'resource:///org/gnome/shell/ui/panelMenu.js';

export default class KeyboardLight extends Extension {
    enable() {
        const repo = GIRepository.Repository.dup_default();
        repo.prepend_search_path(this.path);
        repo.prepend_library_path(this.path);
        const actor = imports.gi.OmniBookKeyboard.indicator_new();
        this.button = new PanelMenu.Button(0, 'Keyboard brightness', true);
        this.button.setSensitive(false);
        this.button.accessible_role = Atk.Role.STATUSBAR;
        this.button.add_child(actor);
        actor.bind_property('visible', this.button.container, 'visible',
            GObject.BindingFlags.SYNC_CREATE);
        Main.panel.addToStatusArea(this.uuid, this.button);
    }

    disable() {
        this.button.destroy();
        this.button = null;
    }
}
