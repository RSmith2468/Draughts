// Astract base class for a pickable object

#ifndef _IPICKABLE_H_
#define _IPICKABLE_H_


class IPickable
{
   public:
      virtual void Execute() = 0;
      virtual ~IPickable() {};   // Destructor needs to be defined as virtual so that a pointer to an IPickable object resolves to the destructor of the derived class
};


#endif // _IPICKABLE_H_
