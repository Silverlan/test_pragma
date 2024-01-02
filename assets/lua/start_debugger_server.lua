--[[
    Copyright (C) 2023 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

if DBG_SERVER_ACTIVE then
	print("Debugger server is already active.")
	return
end
if io == nil then
	console.print_warning('io library not found! Please make sure to add "-luaext" to the launch options of Pragma!')
	console.print_warning("Visit the wiki for more information:")
	console.print_warning("https://wiki.pragma-engine.com/books/lua-api/page/visual-studio-code")
	return
end
DBG_SERVER_ACTIVE = true
local dbg = require("lua-debug"):start("0.0.0.0:12306")
print("Debugger server has been started, waiting for client...")
print("If you don't know what to do, please visit the wiki:")
print("https://wiki.pragma-engine.com/books/lua-api/page/visual-studio-code")
-- dbg : event "wait"
