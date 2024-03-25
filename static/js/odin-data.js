
function read_fp_status()
{
  $.getJSON('/api/' + api_version + '/fp/status/', function(response)
  {
      fp_data = response['value'];
      for (index = 0; index < fp_data.length; index++){
          fp = fp_data[index];
          fp_index = "fp" + (index+1);
          $('#' + fp_index + '-connected').html(led_html(fp['connected'],'green', 20));
          if (fp['connected'] === true || fp['connected'] === 'true'){
              populate_fp_table(fp_index, fp);
          } else {
              empty_fp_table(fp_index);
          }
      }
  });
}

function populate_fp_table(fp_index, fp)
{
    $('#' + fp_index + '-shared-mem').html(led_html(fp['shared_memory']['configured'], 'green', 20));
    $('#' + fp_index + '-hdf-processes').html(fp['hdf']['processes']);
    $('#' + fp_index + '-hdf-rank').html(fp['hdf']['rank']);
    $('#' + fp_index + '-hdf-written').html(fp['hdf']['frames_written'] + " / " + fp['hdf']['frames_max']);
    $('#' + fp_index + '-hdf-file-path').html(fp['hdf']['file_name']);
    $('#' + fp_index + '-writing').html(led_html(fp['hdf']['writing'], 'green', 20));
}

function empty_fp_table(fp_index)
{
    $('#' + fp_index + '-shared-mem').html('');
    $('#' + fp_index + '-hdf-processes').html('');
    $('#' + fp_index + '-hdf-rank').html('');
    $('#' + fp_index + '-hdf-written').html('');
    $('#' + fp_index + '-hdf-file-path').html('');
    $('#' + fp_index + '-writing').html('');
}

function send_fp_command(command, data)
{
    $.ajax({
        url: '/api/' + api_version + '/fp/config/' + command,
        type: 'PUT',
        dataType: 'json',
        data: data,
        headers: {'Content-Type': 'application/json',
                  'Accept': 'application/json'},
        success: null,
        error: process_fp_error
    });
}

function send_fp_reset()
{
    $.ajax({
        url: '/api/' + api_version + '/fp/command/reset_statistics',
        type: 'PUT',
        dataType: 'json',
        headers: {'Content-Type': 'application/json',
                  'Accept': 'application/json'},
        success: null,
        error: process_fp_error
    });
}

function update_fp_params() {
    // Send the number of frames
    send_fp_command('hdf/frames', $('#set-fp-frames').val());
    // Send the path
    send_fp_command('hdf/file/path', $('#set-fp-path').val());
    // Send the name
    send_fp_command('hdf/file/name', $('#set-fp-filename').val());
}

function start_fp_writing()
{
    // Send the write true
    send_fp_reset();
    send_fp_command('hdf/master', 'data');
    send_fp_command('hdf/write', '1');
}

function stop_fp_writing()
{
    // Send the write false
    send_fp_command('hdf/write', '0');
}

function process_fp_error(response)
{
    alert("FAILED: " + response['responseJSON']['error']);
}

$(document).ready(function()
{
    setInterval(read_fp_status, 500);
    $('#fp-start-cmd').on('click', function(event){
      start_fp_writing();
    });
    $('#fp-stop-cmd').on('click', function(event){
      stop_fp_writing();
    });
    $('#set-fp-frames').on('change', function(event){
      update_fp_params();
    });
    $('#set-fp-filename').on('change', function(event){
      update_fp_params();
    });
    $('#set-fp-path').on('change', function(event){
      update_fp_params();
    });
});
