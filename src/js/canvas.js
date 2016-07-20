import UIComponent from './uicomponent';

export default class Canvas extends UIComponent {
    constructor() {
        super();

        this.attach("canvas");

        let context = this.getElement().getContext("2d");

        // TODO: test canvas stacks and event bubbling
    }

}
