import UIComponent from './uicomponent';
import Canvas from './canvas';
import TileCache from './tilecache';

const TILE_CACHE = "TILE_CACHE_KEY";
const DATA_STORE = "DATA_STORE_KEY";

const singletons = new WeakMap();

export default class TissueStack extends UIComponent {
    server=""

    constructor(server = null, parent=null) {
        super();

        let props = new Map();
        props.set(TILE_CACHE, new TileCache());
        singletons.set(this, props);

        if (typeof server === 'string' && server.length !== 0)
            this.server = server;

        this.initUI(parent);
    }

    initUI(parent) {
        if (parent === null) parent = document.body;
        this.attach("TissueStack", parent, "tissuestack");

        new Canvas();
    }

    getTileCache() {
        return singletons.get(this).get(TILE_CACHE);
    }

    destroy() {
        singletons.delete(this);
        this.detach();
    }
}
