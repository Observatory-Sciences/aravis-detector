"""ODIN Aravis camera adapter.

This module implements an odin-control adapter capable of controlling an aravis
camera.

Created on 20th March 2024

:author: Alan Greer, Observatory Sciences Ltd
"""

import logging
import re
from collections import OrderedDict

from odin.adapters.adapter import ApiAdapter, ApiAdapterResponse, response_types
from odin.adapters.parameter_tree import ParameterTree, ParameterTreeError
from odin.util import convert_unicode_to_string
from tornado.escape import json_decode
from aravis_detector.aravis_detector_control import AravisDetectorControl

#from odin_data.control.ipc_channel import IpcChannelException
#from odin_data.control.ipc_tornado_channel import IpcTornadoChannel

#ENDPOINTS_CONFIG_NAME = 'live_view_endpoints'
#COLORMAP_CONFIG_NAME = 'default_colormap'

#DEFAULT_ENDPOINT = 'tcp://127.0.0.1:5020'
#DEFAULT_COLORMAP = "Jet"

DEFAULT_FP_NAME = "fp"


class AravisDetectorAdapter(ApiAdapter):
    """Aravis detector adapter class.

    This class implements the Aravis adapter for odin-control.
    """

    def __init__(self, **kwargs):
        """
        Initialise the adapter.

        Creates an AravisDetectorControl Object that handles the major logic of the adapter.

        :param kwargs: Key Word arguments given from the configuration file,
        which is copied into the options dictionary.
        """
        logging.error("Aravis Adapter init called")
        super(AravisDetectorAdapter, self).__init__(**kwargs)

        self._fp = None
        self._fp_adapter_name = DEFAULT_FP_NAME
        if DEFAULT_FP_NAME in self.options:
            self._fp_adapter_name = self.options[DEFAULT_FP_NAME]

#        if self.options.get(ENDPOINTS_CONFIG_NAME, False):
#            endpoints = [x.strip() for x in self.options.get(ENDPOINTS_CONFIG_NAME, "").split(',')]
#        else:
#            logging.debug("Setting default endpoint of '%s'", DEFAULT_ENDPOINT)
#            endpoints = [DEFAULT_ENDPOINT]
#
#
#        self.live_viewer = LiveViewer(endpoints, default_colormap)
        self.aravis = AravisDetectorControl()


    def initialize(self, adapters):
        if self._fp_adapter_name in adapters:
            self._fp = adapters[self._fp_adapter_name]
            self.aravis.register_fp_adapter(self._fp)
            logging.error("Aravis Detector adapter completed connection to FrameProcessor adapter")

    @response_types('application/json', default='application/json')
    def get(self, path, request):
        """
        Handle a HTTP GET request from a client, passing this to the Live Viewer object.

        :param path: The path to the resource requested by the GET request
        :param request: Additional request parameters
        :return: The requested resource, or an error message and code if the request was invalid.
        """
        try:
            logging.debug("GET request to Aravis controller: {} {}".format(path, request))
            response = self.aravis.get(path, request)
            logging.debug("Response from Aravis controller: {}".format(response))
            content_type = 'application/json'
            status = 200
        except ParameterTreeError as param_error:
            response = {'response': 'LiveViewAdapter GET error: {}'.format(param_error)}
            content_type = 'application/json'
            status = 400

        return ApiAdapterResponse(response, content_type=content_type, status_code=status)

    @response_types('application/json', default='application/json')
    def put(self, path, request):
        """
        Handle a HTTP PUT request from a client, passing it to the Live Viewer Object.

        :param path: path to the resource
        :param request: request object containing data to PUT to the resource
        :return: the requested resource after changing, or an error message and code if invalid
        """
        logging.debug("REQUEST: %s", request.body)
        try:
            data = json_decode(request.body)
            self.aravis.set(path, data)
            #response, content_type, status = self.aravis.get(path)
            response = self.aravis.get(path, request)
            content_type = 'application/json'
            status = 200

        except ParameterTreeError as param_error:
            response = {'response': 'LiveViewAdapter PUT error: {}'.format(param_error)}
            content_type = 'application/json'
            status = 400

        return ApiAdapterResponse(response, content_type=content_type, status_code=status)

    def cleanup(self):
        """Clean up the adapter on shutdown. Calls the Live View object's cleanup method."""
        self.aravis.cleanup()
