async function DELETE(url) {
    try {
        let response = await fetch(url, {
            method: 'DELETE',
        });

        return response.status >= 200 && response.status < 300 ? "OK" : "NOT OK";
    } catch (e) {
        console.error("error occured");
        console.error(e);
    }
}

window.addEventListener("load", function() {

    let cancelBtn = document.querySelector("#cancel-btn");
    let pickBtn = document.querySelector("#pick-btn");
    let deleteBtn = document.querySelector("#delete-btn");
    let filename = '';

    let onCancel = () => {
        console.log('cleared');
        filename = '';
    }

    let onPick = () => {
        const fileinput = document.createElement('input');
        fileinput.type = 'file';

        fileinput.onchange = e => { 
            filename = e.target.files[0].name; 
        }

        fileinput.click();
    }

    async function onDelete() {

        if (filename == '') {
            alert("You need to pick file");
            return ;
        } 

        const reqUri = `${window.location.origin}/delete/${filename}`;
        console.log(reqUri);
        
        let status = await DELETE(reqUri);
        console.log(status);
    }

    cancelBtn.addEventListener('click', onCancel);
    deleteBtn.addEventListener('click', onDelete); 
    pickBtn.addEventListener('click', onPick); 
});
    