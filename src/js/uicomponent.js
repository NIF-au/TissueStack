export default class UIComponent {
    unattachedElement = null;
    element_id = null;

    attach(tag, parent, id) {
        if (typeof tag !== 'string' || tag.length === 0)
            throw new Error("Element needs tag name");

        let el = document.createElement(tag);
        if (!(typeof id === 'string' && id.length > 0) && typeof id !== 'number')
            id = new Date().getTime();

        el.id = id;
        this.element_id = id;

        if (typeof parent === 'string')
            parent = document.getElementById(parent);

        if (parent instanceof Element)
            parent.appendChild(el);
        else
            this.unattachedElement = el;
    }

    getElement() {
        if (this.unattachedElement !== null) return this.unattachedElement;

        return document.getElementById(this.element_id);
    }

    detach() {
        this.unattachedElement = null;
        let myself = this.getElement();
        if (myself)
            myself.parentNode.removeChild(myself);
    }
}
