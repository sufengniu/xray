# Server program
# UDP VERSION


#execfile("udp.py")


host = '127.0.0.1'
port = 2346

from socket import *


def udpRcv(host,port) :

# Set the socket parameters
  #host = "localhost"
  #port = 1101
  buf = 1024
  addr = (host,port)

# Create socket and bind to address
  UDPSock = socket(AF_INET,SOCK_DGRAM)
  UDPSock.bind(addr)

# Receive messages
  while 1:
    data,addr = UDPSock.recvfrom(buf)
    if not data:
        print "Client has exited!"
        break
    else:
        print ByteToHex(data)
	print "  "

# Close socket
  UDPSock.close()





#-------------------------------------------------------------------------------

def ByteToHex( byteStr ):
    """
    Convert a byte string to it's hex string representation e.g. for output.
    """
    
    # Uses list comprehension which is a fractionally faster implementation than
    # the alternative, more readable, implementation below
    #   
    #    hex = []
    #    for aChar in byteStr:
    #        hex.append( "%02X " % ord( aChar ) )
    #
    #    return ''.join( hex ).strip()        

    return ''.join( [ "%02X " % ord( x ) for x in byteStr ] ).strip()

#-------------------------------------------------------------------------------






def udpSend() : 

# Set the socket parameters
  host = "127.0.0.1"
  port = 2346
  buf = 1024
  addr = (host,port)

# Create socket
  UDPSock = socket(AF_INET,SOCK_DGRAM)

  def_msg = "===Enter message to send to server===";
  print "\n",def_msg

# Send messages
  while (1):
    data = raw_input('>> ')
    if not data:
        break
    else:
        if(UDPSock.sendto(data,addr)):
            print "Sending message '",data,"'....."

# Close socket
  UDPSock.close()
# Client program


#udpRcv(host,port) 
