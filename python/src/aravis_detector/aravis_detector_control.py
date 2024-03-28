import json
import logging
import threading
import time

from odin.adapters.parameter_tree import ParameterTree
from odin.adapters.adapter import ApiAdapterRequest

class AravisDetectorControl(object):
    def __init__(self):
        # Underlying FP adapter for controlling the Aravis
        self._fp = None

        self._running = True

        # Camera cached parameters
        self._camera_id = ""
        self._acquisition_mode = ""
        self._frame_rate = 0.0
        self._frame_count = 0
        self._exposure_time = 0.0
        self._pixel_format = None
        self._payload_bytes = 0
        self._streaming = False
        self._frames_captured = 0

        # Setup the parameter tree
        self._parameter_tree = ParameterTree(self.create_parameter_tree())

        # Start up that status and config read thread
        self._update_thread = threading.Thread(target=self.update_loop)
        self._update_thread.start()

    def create_parameter_tree(self):
        params = {
            "config": {
                "mode": (lambda: self._acquisition_mode, self.set_mode),
                "frame_count": (lambda: self._frame_count, self.set_frame_count),
                "frame_rate": (lambda: self._frame_rate, None),
                "exposure_time": (lambda: self._exposure_time, self.set_exposure_time),
                "pixel_format": (lambda: self._pixel_format, None),
                "start_acquisition": (lambda: 0, self.start_acquisition),
                "stop_acquisition": (lambda: 0, self.stop_acquisition)
            },
            "status" : {
                "camera_id": (lambda: self._camera_id, None),
                "streaming": (lambda: self._streaming, None),
                "frames_captured": (lambda: self._frames_captured, None),
                "payload_bytes": (lambda: self._payload_bytes, None)
            }
        }
        return params

    def get(self, path, remove_root=False):
        result = self._parameter_tree.get(path, with_metadata=True)
        return result

    def set(self, path, value):
        self._parameter_tree.set(path, value)

    def register_fp_adapter(self, fp):
        self._fp = fp

    def set_mode(self, mode):
        logging.debug("Setting mode to: {}".format(mode))
        req = ApiAdapterRequest(data=json.dumps({"acquisition_mode": mode}))
        self._fp.put("config/aravis", req)

    def set_frame_count(self, frame_count):
        req = ApiAdapterRequest(data=json.dumps({"frame_count": frame_count}))
        self._fp.put("config/aravis", req)

    def set_exposure_time(self, exposure_time):
        req = ApiAdapterRequest(data=json.dumps({"exposure_time": exposure_time}))
        self._fp.put("config/aravis", req)

    def start_acquisition(self, start_flag):
        logging.debug("Starting acquisition: {}".format(start_flag))
        req = ApiAdapterRequest(data=json.dumps({"start": start_flag}))
        self._fp.put("config/aravis", req)

    def stop_acquisition(self, stop_flag):
        logging.debug("Stopping acquisition: {}".format(stop_flag))
        req = ApiAdapterRequest(data=json.dumps({"stop": stop_flag}))
        self._fp.put("config/aravis", req)

    def read_status(self):
        try:
            req = ApiAdapterRequest(data=json.dumps({}))
            config = self._fp.get("config/aravis", req).data
            status = self._fp.get("status/aravis", req).data
            logging.debug("Reading configuration: {}".format(config))
            logging.debug("Reading status: {}".format(status))
            self._camera_id = status['value'][0]['camera_id']
            self._acquisition_mode = config['value'][0]['acquisition_mode']
            self._frame_rate = config['value'][0]['frame_rate']
            self._frame_count = config['value'][0]['frame_count']
            self._exposure_time = config['value'][0]['exposure_time']
            self._pixel_format = config['value'][0]['pixel_format']
            self._payload_bytes = status['value'][0]['payload']
            self._streaming = status['value'][0]['streaming']
            self._frames_captured = status['value'][0]['frames_made']
        except Exception as ex:
            logging.error("Unable to complete status and configuration read out of FrameProcessor")
            logging.exception(ex)

    def update_loop(self):
        while self._running:
            time.sleep(0.2)
            if self._fp is not None:
                self.read_status()

    def cleanup(self):
        self._running = False
