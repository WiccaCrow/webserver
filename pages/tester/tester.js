async function sendRequest(url, reqmethod, headers = {}, body = '') {
    
    let data = { method: reqmethod };
    if (Object.keys(headers).length !== 0) {
        data.headers = headers;
    }
    if (body !== '') {
        data.body = body;
    }

    try {
        let response = await fetch(url, data);
        return response.status;    
    } catch (e) {
        console.error("An error occured");
        console.error(e);
    }
}

window.addEventListener("load", function() {

    let onCancel = () => {
        filename = '';
        methods.value = 'GET';
        requestBody.value = '';
        requestUri.value = '';
    }

    let onPick = () => {
        const fileinput = document.createElement('input');
        fileinput.type = 'file';

        fileinput.onchange = e => { 
            filename = e.target.files[0].name;
            requestUri.value = '/delete/' + filename;
        }

        fileinput.click();
    }
    
    let methodChanged = () => {
        method = methods.value;
        requestUri.value = '';

        if (method == 'DELETE') {
            pickBtn.hidden = false;
            requestBody.hidden = true;
            requestUri.hidden = false;
            requestUri.disabled = true;
            bodyLabel.hidden = true;
        } else if (method == 'POST' || method == 'PUT') {
            pickBtn.hidden = true;
            requestBody.hidden = false;
            requestUri.hidden = false;
            bodyLabel.hidden = false;
            requestUri.disabled = false;
        } else {
            pickBtn.hidden = true;
            requestBody.hidden = true;
            requestUri.hidden = false;
            bodyLabel.hidden = true;
            requestUri.disabled = false;
        }
    }

    let cancelBtn = document.querySelector("#cancel-btn");
    let pickBtn = document.querySelector("#pick-btn");
    let requestBtn = document.querySelector("#request-btn");
    let requestBody = document.querySelector("#request-body");
    let bodyLabel = document.querySelector("#body-label");
    let requestUri = document.querySelector("#request-uri");
    let methods = document.querySelector("#methods");
    
    
    let filename = '';
    let method = '';
    methodChanged();

    async function onDelete() {

        if (filename == '') {
            alert("You need to pick some file");
            return ;
        } 

        const reqUri = `${requestUri.value}`;
        console.log(reqUri);
        
        let status = await sendRequest(reqUri, method);
        return status;
    }

    async function onGet() {

        const reqUri = `${requestUri.value}`;
        console.log(reqUri);

        let status = await sendRequest(reqUri, method);
        return status;
    }

    async function onPost() {

        const reqUri = `${requestUri.value}`;
        console.log(reqUri);
        
        headers = {};
        if (requestBody.value != '') {
            headers['Content-type'] = 'text/html';
            headers['Content-length'] = requestBody.value.length;
        }

        let status = await sendRequest(reqUri, method, headers, requestBody.value);
        return status;
    }

    async function onOptions() {

        let status = await sendRequest("*", method);
        return status;
    }

    async function onRequest() {

        let res;
        switch (method) {
            case 'GET':
            case 'HEAD':
                res = await onGet();
                break;
            case 'POST':
            case 'PUT':
                res = await onPost();
                break;
            case 'DELETE':
                res = await onDelete();
                break;
            case 'OPTIONS':
                res = await onOptions();
                break;
            default:
                console.error(`Unknown method ${method}`);
                break;
        }
        console.log(`${method} ${requestUri.value} = ${res}`);
        alert(`${method} ${requestUri.value} = ${res}`);
        onCancel();
    }

    cancelBtn.addEventListener('click', onCancel);
    requestBtn.addEventListener('click', onRequest); 
    pickBtn.addEventListener('click', onPick);
    methods.addEventListener('change', methodChanged);
});
    