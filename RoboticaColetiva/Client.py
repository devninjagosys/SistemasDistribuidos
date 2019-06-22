"""
    Train Robot Classes
    TR.AI.NS Project
    Author: Amanda
"""

__all__ = ['Client']

from Protocol import Message, MsgTypes
from enum import Enum


class CliModes(Enum):
    """
        Group of possible client operational states at any given moment
    'login'   -> Has entered the system, but hasn't made a request yet
    'request' -> Client has made a request and is waiting for its answer
    'wait'    -> Request has been accepted and train is on its way
    'moving'  -> In car, going to drop off point
    'dropoff' -> Client has been delivered
    """
    login = 0
    request = 1
    wait = 2
    moving = 3
    dropoff = 4


class Client:
    def __init__(self, ID, pos0, mapFile, destiny, network, log=False):
        self.id = ID

        # Logging variable
        self.log = log

        # Moving attributes
        self.pos = pos0  # Current position of the train
        self.destiny = destiny # Current client destiny
        self.network = network # Connecto to the network communication system
        self.messageBuffer = []

    def step(self):
        # TODO
        pass
    # ---------------------------------------------------
    # Done
    def receive_message(self, msgStr):
        """
        Receives message in string format and converts it into a protocol class
        :param msgStr: Should be a string coded with the message
        """
        msg = Message()
        msg.decode(msgStr)

        if msg.nType == MsgTypes.req_ans:
            self.messageBuffer += [msg]
        else:
            if msg['receiver'] == self.id:
                self.messageBuffer += [msg]
    # ---------------------------------------------------
    # TO DO in Future
    def send_message(self, msg):  
        # TODO
        pass
    # ---------------------------------------------------
    # Done 
    def request_ride(self):
        """
        Send request message to the trains 
        """
        msg_sent = Message(msgType = MsgTypes.req, pickup = self.pos, dropoff=self.destiny)
        self.network.broadcast(msg_sent.encode(), self)
    # ---------------------------------------------------

    def draw(self):
        # TODO
        pass
    # ---------------------------------------------------
    # Done
    def kill(self):
        print("Command for Killing Me")
        del self
