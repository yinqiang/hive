local cell = require "cell"

cell.command {
	ping = function()
		cell.sleep(1)
		return "pong"
	end
}

function cell.main(msg,gui)
	print("=========",win_handle["test"])
	print("pingpong launched",gui[1])
	return msg
end
