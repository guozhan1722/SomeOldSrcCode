function initialize(a,b,r) 
  {
    var latlng = new google.maps.LatLng(a,b);
    var myOptions = {
      zoom: 11,
      center: latlng,
      mapTypeId: google.maps.MapTypeId.ROADMAP,
          streetViewControl: true
    };
    var map = new google.maps.Map(document.getElementById("map_canvas"), myOptions);

    geocoder = new google.maps.Geocoder();
         var address = a + ', ' + b;
    if (geocoder) {
        var lat = parseFloat(a);                         
        var lng = parseFloat(b);                                           
        var latlng = new google.maps.LatLng(lat, lng);                  
        geocoder.geocode({'latLng': latlng}, function(results, status) {
        if (status == google.maps.GeocoderStatus.OK) {
        if (results[1]) {
          map.setZoom(11);
          marker = new google.maps.Marker({
              position: latlng,
              map: map,
              title: 'The Closest Cell Tower'
          });

          // Add circle overlay and bind to marker
          var circle = new google.maps.Circle({
              map: map,
              radius: r,    // metres
              fillColor: '#0000FF'
                });
          circle.bindTo('center', marker, 'position');
          infowindow.setContent(results[1].formatted_address);
          infowindow.open(map, marker);
        }                                                        
        } else {                                                        
         // alert("Not successful for the following reason: " + status);
        var latlng2 = new google.maps.LatLng(a,b);
        var myOptions2 = {
      zoom: 11,                                
      center: latlng2,                         
      mapTypeId: google.maps.MapTypeId.ROADMAP,
          streetViewControl: true
    };                          
     map.setOptions(myOptions2);
        }
      });
    } 
}


