async function sendRequest(url, reqmethod, headers = {}, body = '') {
    
    try {
        let response = await fetch(url, {
            method: reqmethod,
            headers: headers,
            body: body
        });
        return response.status;    
    } catch (e) {
        console.error("An error occured");
        console.error(e);
    }
}

async function sendRequest(url, reqmethod) {
    
    try {
        let response = await fetch(url, {
            method: reqmethod,
        });
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
        }

        fileinput.click();
    }
    
    let methodChanged = () => {
        method = methods.value;

        if (method == 'DELETE') {
            pickBtn.hidden = false;
            requestBody.hidden = true;
            requestUri.hidden = true;
            bodyLabel.hidden = true;
        } else if (method == 'POST' || method == 'PUT') {
            pickBtn.hidden = true;
            requestBody.hidden = false;
            requestUri.hidden = false;
            bodyLabel.hidden = false;
        } else {
            pickBtn.hidden = true;
            requestBody.hidden = true;
            requestUri.hidden = false;
            bodyLabel.hidden = true;
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
            alert("You need to pick file");
            return ;
        } 

        const reqUri = `/delete/${filename}`;
        console.log(reqUri);
        
        let status = await sendRequest(reqUri);
        console.log(status);
    }

    async function onGet() {

        const reqUri = `${requestUri.value}`;
        console.log(reqUri);

        let status = await sendRequest(reqUri, method);
        console.log(status);
        return status;
    }

    async function onPost() {

        const reqUri = `${requestUri.value}`;
        console.log(reqUri);
        
        headers = {};
        if (requestBody.value != '') {
            headers['Content-type'] = 'text/html';
        }

        let status = await sendRequest(reqUri, method, headers, requestBody.value);
        console.log(status);
        return status;
    }

    async function onOptions() {

        let status = await sendRequest("*", method, headers, requestBody.value);
        console.log(status);
        return status;
    }

    async function onRequest() {

        let res;
        switch (method) {
            case 'GET':
            case 'POST':
            case 'PUT':
            case 'HEAD':
                res = await onGet();
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
        onCancel();
        alert(`Returned status: ${res}`);
    }

    cancelBtn.addEventListener('click', onCancel);
    requestBtn.addEventListener('click', onRequest); 
    pickBtn.addEventListener('click', onPick);
    methods.addEventListener('change', methodChanged);
});
    