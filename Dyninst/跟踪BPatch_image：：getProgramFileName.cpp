name = img->getProgramFileName(name, 49);
/**************************************************
 * dyninst-8.0\dyninstAPI\src\BPatch_image.C
 **************************************************/
char *BPatch_image::getProgramNameInt(char *name, unsigned int len) 
{
   std::vector<AddressSpace*> as;
   (BPatch_addressSpace*)BPatch_image::addSpace->getAS(as);
   AddressSpace *aout = as[0];
   const char *imname =  aout->getAOut()->fullName().c_str();
   ...
}

/**************************************************
 * dyninst-8.0\dyninstAPI\src\BPatch_process.C
 **************************************************/
void BPatch_process::getAS(std::vector<AddressSpace *> &as)
{
   as.push_back(static_cast<AddressSpace*>((PCProcess*)BPatch_process::llproc));
}

/**************************************************
 * dyninst-8.0\dyninstAPI\src\addressSpace.h
 **************************************************/
mapped_object *AddressSpace::getAOut()
{
	return (mapped_object *)mapped_objects[0];
}

/**************************************************
 * dyninst-8.0\dyninstAPI\src\mapped_object.h
 **************************************************/
const string &mapped_object::fullName() const
{
	return (string)mapped_object::fullName_;
}

//-------------------------------------------------
char *BPatch_image::getProgramNameInt(char *name, unsigned int len)
{
	return ((BPatch_process*)img->addSpace)->llproc->mapped_objects[0]->fullName_;
}