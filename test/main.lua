local cell = require "cell"
local msgpack = require "cell.msgpack"
local binlib =  require "cell.binlib"
local function accepter(fd, addr, listen_fd)
	print("Accept from ", listen_fd)
	-- can't read fd in this function, because socket.cell haven't forward data from fd
	local client = cell.cmd("launch", "test.client",fd, addr)
	-- return cell the data from fd will forward to, you can also return nil for forwarding to self
	return client
end

local gui = sraw.create()
gui[1]="test"
local p = msgpack.pack({"msgpack",2})
local up =msgpack.unpack(p)
print(333333333)
print(up)
print(up[1])

print("--------",win_handle)
win_handle["test"] = "win_handle hello world"
local function udp(fd,len,msg,peer_ip,peer_port)
      local obj=cell.bind(fd)
      obj:write(p,peer_ip,peer_port)
      print("receive from ",peer_ip,peer_port)

end

function cell.main()
	print("[cell main]",cell.self)
	-- save listen_fd for prevent gc.
	cell.listen("127.0.0.1:8888",accepter)
	cell.open(9998,udp)
--[[
	local sock = cell.connect("localhost", 8088)	
	local u =msgpack.pack({"test","pwd"})
	local len = string.len(u)+2
	local u1 = binlib.pack(">ISA",len,1,u)
	sock:write(u1)
	print(u1)
	local t1 =sock:readbytes(4)	
	_,len=binlib.unpack(">I",t1)
	print(len)       
	local r = sock:readbytes(len)
	local str_len = len -2
	local pos,cmd,rep = binlib.unpack(">SA"..str_len,r)
	print(cmd)
	print(pos)
	print(string.len(rep))
	local rep = msgpack.unpack(rep)
	print(rep[1],rep[2])
]]
	print(cell.cmd("echo","Hello world"))
	local ping, pong = cell.cmd("launch", "test.pingpong","pong",gui)
	print(ping,pong)
	print(cell.call(ping, "ping"))
	cell.fork(function()
		-- kill ping after 9 second
		cell.sleep(900)
		cell.cmd("kill",ping) end
	)
	for i=1,1 do
		print(pcall(cell.call,ping, "ping"))
		cell.sleep(100)
		print(i)
	end
--	cell.exit()
end
