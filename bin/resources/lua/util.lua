function getArrayLength(array)
	return #array
end

function pushArray(array, value)
	array[getArrayLength(array) + 1] = value
end
