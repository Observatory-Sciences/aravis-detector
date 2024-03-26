api_version = '0.1';
monitor_names = [];
monitor_desc = {};
current_page = "home-view";

aravis = {
    api_version: '0.1',
    current_page: '.home-view',
    status: {}
};

aravis_modes = [
  "SingleFrame",
  "MultiFrame",
  "Continuous"
]

String.prototype.replaceAll = function(search, replacement) {
    var target = this;
    return target.replace(new RegExp(search, 'g'), replacement);
};

$.put = function(url, data, callback, type)
{
  if ( $.isFunction(data) ){
    type = type || callback,
    callback = data,
    data = {}
  }

  return $.ajax({
    url: url,
    type: 'PUT',
    dataType: 'json',
    data: JSON.stringify(data),
    headers: {'Content-Type': 'application/json',
              'Accept': 'application/json'},
    success: callback,
    contentType: type
  });
}


$( document ).ready(function() 
{
  update_api_version();
  update_api_adapters();
  update_server_setup();
  update_aravis();
  render(decodeURI(window.location.hash));

  setInterval(update_server_setup, 1000);
  setInterval(update_aravis, 200);


  // Configuration items
  $('#set-exposure').on('change', function(event){
    write_exposure();
  });
  $('#set-mode').on('change', function(event){
    write_mode();
  });
  $('#set-period').on('change', function(event){
    write_acq_period();
  });
  $('#set-num-frames').on('change', function(event){
    write_num_frames();
  });
  $('#start-acquisition').on('click', function(event){
    start_acquisition();
  });
  $('#stop-acquisition').on('click', function(event){
    stop_acquisition();
  });

  $(window).on('hashchange', function(){
		// On every hash change the render function is called with the new hash.
		// This is how the navigation of the app is executed.
		render(decodeURI(window.location.hash));
	});
});

function write_exposure()
{
    val = $('#set-exposure').val();
    $.put('/api/' + api_version + '/aravis/config/exposure_time', parseFloat(val), function(response){});
}

function write_mode()
{
    val = $('#set-mode').val();
    $.put('/api/' + api_version + '/aravis/config/mode', aravis_modes[val], function(response){});
}

function write_num_frames()
{
    val = $('#set-num-frames').val();
    $.put('/api/' + api_version + '/aravis/config/frame_count', parseFloat(val), function(response){});
}

function start_acquisition()
{
  $.put('/api/' + api_version + '/aravis/config/start_acquisition', 1, function(response){});
}

function stop_acquisition()
{
  $.put('/api/' + api_version + '/aravis/config/stop_acquisition', 1, function(response){});
}

function process_cmd_response(response)
{
}

function update_api_version() {

    $.getJSON('/api', function(response) {
        $('#api-version').html(response.api);
        aravis.api_version = response.api;
    });
}

function update_api_adapters() {
    $.getJSON('/api/' + api_version + '/adapters/', function(response) {
        adapter_list = response.adapters.join(", ");
        $('#api-adapters').html(adapter_list);
    });
}

function update_aravis() {
    $.getJSON('/api/' + api_version + '/aravis', function(response) {
        $('#camera-id').html(response.status.camera_id.value);
        $('#get-exposure').html(response.config.exposure_time.value);
        $('#get-rate').html(response.config.frame_rate.value);
        $('#get-num-frames').html(response.config.frame_count.value);
        $('#get-mode').html(response.config.mode.value);
        mode = response.config.mode.value
        for (i=0; i < aravis_modes.length; i++){
          if (mode == aravis_modes[i]){
            $('#set-mode').val(""+i);
          }
        }
        $('#get-pixel-format').html(response.config.pixel_format.value);
        $('#get-acq-state').html(led_html(response.status.streaming.value,'green', 20));
        $('#get-frames-captured').html(response.status.frames_captured.value);
    });
}
  
function update_server_setup() {
    $.getJSON('/api/' + api_version + '/sys', function(response) {
        $('#odin-version').html(response.odin_version);
        $('#server-up-time').html(response.server_uptime);
        $('#system-type').html(response.platform.system);
        $('#processor-type').html(response.platform.processor);
        $('#system-release').html(response.platform.release);
        $('#system-version').html(response.platform.version);
        $('#python-version').html(response.python_version);
        $('#tornado-version').html(response.tornado_version);
    });
}

function led_html(value, colour, width)
{
  var html_text = "<img width=" + width + "px src=img/";
  if (value == 0){
    html_text += "led-off";
  } else {
    html_text +=  colour + "-led-on";
  }
  html_text += ".png></img>";
  return html_text;
}

function render(url) 
{
  // This function decides what type of page to show 
  // depending on the current url hash value.
  // Get the keyword from the url.
  var temp = "." + url.split('/')[1];
  if (url.split('/')[1]){
    document.title = "Aravis (" + url.split('/')[1] + ")";
  } else {
    document.title = "Aravis";
  }
  // Hide whatever page is currently shown.
  $(".page").hide();
		
  // Show the new page
  $(temp).show();
  current_page = temp;
    
  if (temp == ".home-view"){
    update_api_version();
    update_api_adapters();
  }
}
