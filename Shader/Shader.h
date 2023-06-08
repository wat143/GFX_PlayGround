#ifndef __SHADER__
#define __SHADER__

class Shader {
 protected:
  const char* vshaderSource;
  const char* fshaderSource;
 public:
  Shader(const char* vshader, const char* fshader):
    vshaderSource(vshader),fshaderSource(fshader){}
  virtual ~Shader(){}
  virtual void initShader()=0;
  virtual void useProgram()=0;
  virtual unsigned int getAttribLocation(char*)=0;
  virtual unsigned int getUniLocation(char*)=0;
};

#endif
