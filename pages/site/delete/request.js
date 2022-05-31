class httpAPI {

    delete(url) {  
        fetch(url, {
            method: 'DELETE',
        })
        .then(response => {
            return response.status == 204 ? "OK" : "NOT OK";
        });
    }

    checkStatus (response) {
        if (response.ok) {
          return response;
        }
    
        return response.json().then(error => {
          error.response = response;
          throw error;
        });
    }
}


window.addEventListener("load", function() {

    let cancelBtn = document.querySelector("#cancel-btn");
    let filepicker = document.querySelector("#filepicker");
    let uriInput = document.querySelector("#uri");
    let deleteBtn = document.querySelector("#delete-btn");
    
    filepicker.defaultValue = '/src.png';
    uriInput.defaultValue = 'http://localhost:7676/';
    let onCancel = () => {
        console.log('cleared');
        filepicker.value = '';
    }

    function onDelete() {
        let filename = filepicker.value.replace(/.*[\/\\]/, '');
        let uri = uriInput.value;
        if (uri[uri.length - 1] == '/')
            uri = uri.slice(0, -1);

        console.log(`${uri}/${filename}`);
        const api = new httpAPI;
        
        let status = api.delete(`${uri}/${filename}`);
        console.log(status);
        // .then(data => console.log(data))
        // .catch(err => console.log(err));
    }

    cancelBtn.addEventListener('click', onCancel);
    deleteBtn.addEventListener('click', onDelete); 
});
    