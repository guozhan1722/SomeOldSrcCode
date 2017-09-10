function clearAuthenticationCache(page) 
{
    // Default to a non-existing page (give error 500).
    // An empty page is better, here.
    if (!page) 
        page = '.force_logout';

    try {
        var agt=navigator.userAgent.toLowerCase();
        if (agt.indexOf("msie") != -1) {
            // IE clear HTTP Authentication
            document.execCommand("ClearAuthenticationCache");
        }

        //var is_chrome = navigator.userAgent.toLowerCase().indexOf('chrome') > -1;
        //var xhr = new ();
        //xhr.onreadystatechange = handleStateChange; // Implemented elsewhere.
        //xhr.open("GET", chrome.extension.getURL('/config_resources/config.json'), true);
        //xhr.send();
        else if (agt.indexOf("chrome") != -1) {
            // Let's create an xmlhttp object
            //document.write("Are you sure you want to log out?<br>To log out, you must close Web Browser");
        } else {
            // Let's create an xmlhttp object
            var xmlhttp = createXMLObject();
            // Let's prepare invalid credentials
            xmlhttp.open("GET", page, true, "logout", "logout");
            // Let's send the request to the server
            xmlhttp.send("");
            // Let's abort the request
            xmlhttp.abort();
        }
    } 

    catch(e) {
        // There was an error
        return;
    }
}
    
function createXMLObject() 
{
    try {
        if (window.XMLHttpRequest) {
            xmlhttp = new XMLHttpRequest();
        }
        // code for IE
        else if (window.ActiveXObject) {
            xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
        }
    } 

    catch (e) {
        xmlhttp=false
    }

    return xmlhttp;
}

function logoutUser() 
{
    setTimeout('location.reload(true)', 1000);
    xmlhttp = GetXmlHttpObject();
    if (xmlhttp==null) {
        return;
    }
    //alert(xmlhttp);
    var url = "index.php";

    xmlhttp.open("GET", url, true, "dummy_user", "dummy_password");

    xmlhttp.setRequestHeader( "If-Modified-Since", "Sat, 1 Jan 2000 00:00:00 GMT" );
    xmlhttp.setRequestHeader( 'Accept', 'message/x-formresult' );   
    xmlhttp.send(null);
}


function GetXmlHttpObject()
{
    if (window.XMLHttpRequest)
    {
        // code for IE7+, Firefox, Chrome, Opera, Safari
        return new XMLHttpRequest();
    }

    if (window.ActiveXObject)
    {
        // code for IE6, IE5
        return new ActiveXObject("Microsoft.XMLHTTP");
    }

    return null;
}














