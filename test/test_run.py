"""
# Integration test

Starts the frame Processor app from Odin-Data then checks config and status messages using
odin-data tools

Because this is an integration test, to properly run it the project files have to be in the
right folder structure and installed.

Note: the aravis-detector directory can now also be the source dir.

The two structures are both valid:

Source_dir
    - aravis-detector
        - test
    - prefix
    - ...

Source_dir
    - test
    - prefix
    - ...

Make sure both the Frame processor and fake camera are installed in prefix/bin

If you follow the build instructions you should already have all of this installed

"""

from odin_data.control.ipc_channel import IpcChannel
from odin_data.control.ipc_message import IpcMessage
import subprocess
import pytest  # noqa: F401
import json  # noqa: F401
import sys  # noqa: F401
import os

# noqa tells flake8 to ignore unused imports for now, don't remove


class TestIntegration:
    """
    Tests AravisDetector plugin integration in the odin data

    Starts an instance of the frameProcessor app and the arv_fake_camera
    Then it uses the odin-data Ipc messages to communicate with it.
    """
    # Find the paths for prefix and configs
    current_dir = os.path.dirname(os.path.realpath(__file__))  # source_dir/aravis-detector/test
    plugin_dir, test_tail = os.path.split(current_dir)  # source_dir/aravis-detector
    source_dir, arv_tail = os.path.split(plugin_dir)  # source_dir

    # Double check that the file exists and try different path
    if not os.path.isfile(f"{source_dir}/prefix/bin/frameProcessor"):
        # if the install was a done in the same directory /aravis-detector/ this will
        # change paths so it only goes on directory above /test/
        current_dir = os.path.dirname(os.path.realpath(__file__))  # source_dir/test
        source_dir, arv_tail = os.path.split(current_dir)  # source_dir

        for file in ["/bin/frameProcessor", "/lib/libAravisDetectorPlugin.so"]:
            if not os.path.isfile(f"{source_dir}/prefix{file}"):
                raise FileNotFoundError(f"Test setup process failed to find {file} in the source \
                                        directory: {source_dir} or {os.path.split(plugin_dir)}")

    # change the config file to the correct path
    with open(f"{current_dir}/test_plugin.json") as config_file:
        fp_configs = json.load(config_file)
        fp_configs[0]["plugin"]["load"]["library"] = f"{source_dir}" +\
                                                     "/prefix/lib/libAravisDetectorPlugin.so"
    with open(f"{current_dir}/test_plugin.json", "w") as config_file:
        json.dump(fp_configs, config_file)

    # start fp and camera
    fp_app = subprocess.Popen([
            f"{source_dir}/prefix/bin/frameProcessor",
            "--ctrl", "tcp://0.0.0.0:5004",
            "--config", f"{current_dir}/test_plugin.json"
        ])
    fake_cam = subprocess.Popen([
        f"{source_dir}/prefix/bin/arv-fake-gv-camera-0.8",
        "-s", "GV02", "-d", "all"
    ])

    # Ipc Message stuff
    _id = 0
    ctrl_channel = IpcChannel(IpcChannel.CHANNEL_TYPE_DEALER)
    ctrl_channel.connect('tcp://127.0.0.1:5004')
    status = {}
    config = {}

    def _next_id(self):
        """Increase the ipc message id by one"""
        self._id += 1
        return (self._id)

    def _get_reply(self, wait_time: int = 1000):
        """
        Query the Ipc Channel for a reply

        Starts a channel poll. If after wait_time ms it doesn't get a reply it tries once more

        Args:
            wait_time (int, optional): maximum wait time for an answer in milliseconds.
              Defaults to 1000.

        Returns:
            reply: Ipc Message object with info from the frame processor

        Raises:
            TimeoutError: when the response takes longer than expected
        """
        poll_reply = self.ctrl_channel.poll(wait_time)
        if poll_reply == IpcChannel.POLLIN:
            reply = IpcMessage(from_str=self.ctrl_channel.recv())
            return reply
        else:
            poll_reply = self.ctrl_channel.poll(wait_time)
            if poll_reply == IpcChannel.POLLIN:
                reply = IpcMessage(from_str=self.ctrl_channel.recv())
                return reply
            else:
                raise TimeoutError("Response not received")

    def _send_config(self, param_name: str, param_val):
        """
        Send config parameter to connect to the fake camera

        Checks the reply for errors and confirms the connected camera
        has the intended ip address.
        """
        config_msg = IpcMessage('cmd', 'configure', id=self._next_id())
        config_msg.set_param(param_name='aravis', param_value={param_name: param_val})
        self.ctrl_channel.send(config_msg.encode())

    def test_start(self):
        """
        print start
        """
        print(self.fp_app)

    def test_status(self):
        """
        Test status response from plugin

        Sends a status request then checks the response
         - checks if response is status val
         - if 'aravis' is a parameter
        If correct then the AD plugin has been loaded and status msgs work

        """
        status_msg = IpcMessage('cmd', 'status', id=self._next_id())
        self.ctrl_channel.send(status_msg.encode())
        self.status = self._get_reply()
        if self.status.get_msg_type() == 'nack':
            assert 0
        if self.status.get_msg_val() == "status":
            params = self.status.get_params()
            assert 'aravis' in params
        else:
            assert self.status.get_msg_val() == "status"

    def test_config(self):
        """
        Test config response from plugin

        Sends a config request then checks the response
         - checks if response is config val
         - if 'aravis' is a parameter
        If correct then the AD plugin has been loaded and config msgs work

        """
        config_msg = IpcMessage('cmd', 'request_configuration', id=self._next_id())
        self.ctrl_channel.send(config_msg.encode())
        self.config = self._get_reply()
        assert self.config.get_msg_val() == "request_configuration"

    def test_connect_camera(self):
        """
        Send config parameter to connect to the fake camera

        Checks the reply for errors and confirms the connected camera
        has the intended ip address.
        """
        self._send_config("ip_address", "127.0.0.1")
        reply = self._get_reply(5000)
        if reply.get_msg_type() == "ack":
            status_msg = IpcMessage('cmd', 'status', id=self._next_id())
            self.ctrl_channel.send(status_msg.encode())
            self.status = self._get_reply()
            status_values = self.status.get_param("aravis")
            assert status_values['camera_ip'] == '127.0.0.1'
        else:
            assert reply.get_msg_type() == "ack"

    def test_bad_config_response(self):
        """
        Sends an invalid config message

        Expects nack response
        message attempts to set frame rate negative
        """
        self._send_config("frame_rate", -10)
        reply = self._get_reply(5000)
        assert reply.get_msg_type() == "nack"

    def test_shutdown(self):
        """
        Sends shutdown request and checks for answer
        """
        shutdown_msg = IpcMessage('cmd', 'shutdown', id=self._next_id())
        self.ctrl_channel.send(shutdown_msg.encode())
        reply = self._get_reply()
        assert reply.get_msg_val() == "shutdown"

    def __del__(self):
        """Explicitly stop the apps"""
        self.fake_cam.kill()
        self.fp_app.kill()


if __name__ == "__main__":
    ap = TestIntegration()
    ap.test_start()
    ap.test_config()
    ap.test_status()
    ap.test_connect_camera()
    ap.test_shutdown()
