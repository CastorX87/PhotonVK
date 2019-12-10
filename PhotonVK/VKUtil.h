#pragma once
#ifndef _VKUTIL_H_
#define _VKUTIL_H_

#include "Util.h"

namespace vku
{
	vk::ShaderModule CreateShaderModule(vk::Device& vkDevice, const std::vector<char>& code)
	{
		vk::ShaderModuleCreateInfo smci;
		smci.codeSize = code.size();
		smci.pCode = reinterpret_cast<const uint32_t*>(code.data());

		return vkDevice.createShaderModule(smci);
	}
}

#endif