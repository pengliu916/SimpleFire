#pragma once

#include <D3D11.h>
#include <xnamath.h>
#include "DXUT.h"
#include "Utility.h"
#include "DXUT\Optional\DXUTcamera.h"
#include "DXUT\Optional\SDKmisc.h"
#include "DXUT\Optional\SDKMesh.h"

#ifndef SUB_TEXTUREWIDTH
#define SUB_TEXTUREWIDTH 640
#endif

#ifndef SUB_TEXTUREHEIGHT
#define SUB_TEXTUREHEIGHT 480
#endif

#define MAX_PARTICLES 300000


using namespace std;

struct SF_ConstBuffer
{
	XMMATRIX	mWorldViewProj;
	XMMATRIX	mWorldView;
	XMMATRIX	mInvWorldView;
	XMMATRIX	mTeapot;
	XMMATRIX	mSphere;
	XMFLOAT4	vGravity;

	float		fGlobalTime;
	float		fElapsedTime;
	float		fFireInterval;
	float		fMaxDrops;
	
	float		fDropInitVel;
	float		fDropLife;
	float		fDropLengthFactor;
	float		fSphereRadius;
	
	XMFLOAT4	vSpherePos;
	XMFLOAT4	vLightPos;
	XMFLOAT4	vLightCol;

	float		fGroundHeight;
};

struct PARTICLE_V
{
	XMFLOAT3	vPos;
	XMFLOAT3	vVel;
	FLOAT		fTimer;
	UINT		uType;
};

class SimpleFire
{
public:

	D3D11_VIEWPORT					m_RTviewport;
	CModelViewerCamera				m_Camera;

	ID3D11VertexShader*				m_pRenderParticleVS;
	ID3D11PixelShader*				m_pRenderParticlePS;
	ID3D11GeometryShader*			m_pRenderParticleGS;
	
	ID3D11VertexShader*				m_pAdvanceParticleVS;
	ID3D11GeometryShader*			m_pAdvanceParticleGS;
	ID3D11InputLayout*				m_pParticlesVL;
	
	ID3D11VertexShader*				m_pTeapotVS;
	ID3D11VertexShader*				m_pSphereVS;
	ID3D11VertexShader*				m_pGroundVS;
	ID3D11GeometryShader*			m_pGroundGS;
	ID3D11PixelShader*				m_pGeneralMeshPS;
	ID3D11InputLayout*				m_pGeneralVL;

	ID3D11Buffer*					m_pParticleStartVB; 
	ID3D11Buffer*					m_pParticleStreamToVB; 
	ID3D11Buffer*					m_pParticleDrawFromVB; 

	ID3D11ShaderResourceView*		m_pSparkTexSRV;
	ID3D11ShaderResourceView*		m_pFlameTexSRV;

	ID3D11SamplerState*				m_pGeneralTexSS;
	ID3D11SamplerState*				m_pFlameTexSS;

	ID3D11ShaderResourceView*		m_pRandomTexSRV;
	ID3D11Texture1D*				m_pRandomTex1D;	

	//For Texture output
	ID3D11Texture2D*				m_pOutputTexture2D;
	ID3D11Texture2D*				m_pOutputStencilTexture2D;
	ID3D11RenderTargetView*			m_pOutputTextureRTV;
	ID3D11ShaderResourceView*		m_pOutputTextureSRV;
	ID3D11DepthStencilView*			m_pOutputStencilView;
	// The following allow transparency happened only on sth you want
	ID3D11BlendState*				m_pTransparentOnBS;
	ID3D11BlendState*				m_pTransparentOffBS;
	// The following depth stencil state fix the artifact when you have lots spirits while still want depth test
	ID3D11DepthStencilState*		m_pDSparticleState;
	ID3D11DepthStencilState*		m_pDSsceneState;

	SF_ConstBuffer					m_CBallInOne;
	ID3D11Buffer*					m_pCBallInOne;

	UINT							m_rendertargetWidth;
	UINT							m_rendertargetHeight;

	CDXUTSDKMesh					m_TeapotMesh;
	CDXUTSDKMesh					m_SphereMesh;

	XMVECTOR						m_vLightColor;
	bool			m_bFirst;

	SimpleFire(UINT width = SUB_TEXTUREWIDTH, UINT height = SUB_TEXTUREHEIGHT)
	{
		m_rendertargetWidth=width;
		m_rendertargetHeight=height;
		m_pOutputTexture2D=NULL;
		m_pOutputTextureRTV=NULL;
		m_pOutputTextureSRV=NULL;
		m_bFirst = true;
		
		m_CBallInOne.fFireInterval = 0;
		m_CBallInOne.fMaxDrops = 100;
		m_CBallInOne.fDropInitVel = 40;
		m_CBallInOne.fDropLife = 1.5;
		m_CBallInOne.fDropLengthFactor = 3.5;
		m_CBallInOne.fSphereRadius = 0.7;
		m_CBallInOne.vSpherePos = XMFLOAT4(1.4,-1.5,1.5,1);
		m_CBallInOne.vLightPos = XMFLOAT4(0,0,0,0);
		m_CBallInOne.vLightCol = XMFLOAT4(1, 1, 1, 1);
		m_vLightColor = XMLoadFloat4( &XMFLOAT4(1, 0.5, 0, 1) );
		//m_CBallInOne.mTeapot = XMMatrixTranspose( XMMatrixTranslation(-1,1,0));
		m_CBallInOne.mTeapot = XMMatrixTranspose( XMMatrixTranslation(0,0,0));
		m_CBallInOne.mSphere = XMMatrixTranspose( XMMatrixTranslationFromVector(XMLoadFloat4(&m_CBallInOne.vSpherePos)));
		m_CBallInOne.fGroundHeight = -2.2f;
	}

	HRESULT Initial()
	{
		HRESULT hr=S_OK;
		
		return hr;
	}

	HRESULT CreateResource(ID3D11Device* pd3dDevice)
	{
		HRESULT hr=S_OK;

		ID3DBlob* pVSBlob = NULL;
		wstring filename=L"SimpleFire.fx";

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TIMER", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		V_RETURN(CompileShaderFromFile((WCHAR*)filename.c_str(),"RenderVS","vs_5_0",&pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pRenderParticleVS));
		V_RETURN(CompileShaderFromFile((WCHAR*)filename.c_str(),"AdvanceVS","vs_5_0",&pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pAdvanceParticleVS));
		V_RETURN(pd3dDevice->CreateInputLayout(layout,ARRAYSIZE(layout),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pParticlesVL));

		D3D11_INPUT_ELEMENT_DESC layoutMesh[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		V_RETURN(CompileShaderFromFile((WCHAR*)filename.c_str(),"GroundVS","vs_5_0",&pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pGroundVS));
		V_RETURN(CompileShaderFromFile((WCHAR*)filename.c_str(),"TeapotVS","vs_5_0",&pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pTeapotVS));
		V_RETURN(CompileShaderFromFile((WCHAR*)filename.c_str(),"SphereVS","vs_5_0",&pVSBlob));
		V_RETURN(pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),NULL,&m_pSphereVS));
		V_RETURN(pd3dDevice->CreateInputLayout(layoutMesh,ARRAYSIZE(layoutMesh),pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(),&m_pGeneralVL));
		pVSBlob->Release();

		ID3DBlob* pGSBlob = NULL;
		V_RETURN(CompileShaderFromFile(L"SimpleFire.fx","RenderGS","gs_5_0",&pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(),NULL,&m_pRenderParticleGS));
		V_RETURN(CompileShaderFromFile(L"SimpleFire.fx","GroundGS","gs_5_0",&pGSBlob));
		V_RETURN(pd3dDevice->CreateGeometryShader(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(),NULL,&m_pGroundGS));
		V_RETURN(CompileShaderFromFile(L"SimpleFire.fx","AdvanceGS","gs_5_0",&pGSBlob));
		D3D11_SO_DECLARATION_ENTRY pDecl[] =
		{
			{ 0, "POSITION", 0, 0, 3, 0 },   
			{ 0, "NORMAL", 0, 0, 3, 0 },    
			{ 0, "TIMER", 0, 0, 1, 0 },     
			{ 0, "TYPE", 0, 0, 1, 0 }, 
		};
		UINT stride = 7 * sizeof( float ) + 1 * sizeof( UINT );
		UINT elems = sizeof( pDecl ) / sizeof( D3D11_SO_DECLARATION_ENTRY );

		V_RETURN(pd3dDevice->CreateGeometryShaderWithStreamOutput(pGSBlob->GetBufferPointer(),pGSBlob->GetBufferSize(), pDecl, elems, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, NULL, &m_pAdvanceParticleGS ));
		pGSBlob->Release();

		ID3DBlob* pPSBlob = NULL;
		V_RETURN(CompileShaderFromFile(L"SimpleFire.fx","RenderPS","ps_5_0",&pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pRenderParticlePS));
		
		V_RETURN(CompileShaderFromFile(L"SimpleFire.fx","MeshPS","ps_5_0",&pPSBlob));
		V_RETURN(pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),pPSBlob->GetBufferSize(),NULL,&m_pGeneralMeshPS));
		pPSBlob->Release();

		// Load teapot mesh
		V_RETURN( m_TeapotMesh.Create( pd3dDevice, L"Teapot.sdkmesh" ));
		V_RETURN( m_SphereMesh.Create( pd3dDevice, L"Ball.sdkmesh" ));


		// Create the vertex buffer
		D3D11_BUFFER_DESC bd =
		{
			3 * sizeof( PARTICLE_V ),
			D3D11_USAGE_DEFAULT,
			D3D11_BIND_VERTEX_BUFFER,
			0,
			0
		};

		D3D11_SUBRESOURCE_DATA vbInitData;
		ZeroMemory( &vbInitData, sizeof( D3D11_SUBRESOURCE_DATA ));  

		PARTICLE_V vertStart[3] =
		{
			{ XMFLOAT3( 1.5, 0.35, 0 ), XMFLOAT3( 0, 1.5, 0 ),float( 0 ), UINT( 0 ) },
			{ XMFLOAT3( 1.5, 0.35, 0 ), XMFLOAT3( 0, 1.5, 0 ),float( 0 ), UINT( 0 ) },
			{ XMFLOAT3( 1.5, 0.35, 0 ), XMFLOAT3( 0, 1.5, 0 ),float( 0 ), UINT( 0 ) },
		};

		vbInitData.pSysMem = vertStart;
		vbInitData.SysMemPitch = sizeof( PARTICLE_V );

		V_RETURN( pd3dDevice->CreateBuffer( &bd, &vbInitData, &m_pParticleStartVB ) );

		bd.ByteWidth = MAX_PARTICLES * sizeof( PARTICLE_V );
		bd.BindFlags |= D3D11_BIND_STREAM_OUTPUT;
		V_RETURN( pd3dDevice->CreateBuffer( &bd, NULL, &m_pParticleStreamToVB ) );
		V_RETURN( pd3dDevice->CreateBuffer( &bd, NULL, &m_pParticleDrawFromVB ) );
		DXUT_SetDebugName( m_pParticleStreamToVB, "m_pParticleStreamToVB");
		DXUT_SetDebugName( m_pParticleDrawFromVB, "m_pParticleDrawFromVB");

		// Create the constant buffers
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0    ;
		bd.ByteWidth = sizeof(SF_ConstBuffer);
		V_RETURN(pd3dDevice->CreateBuffer( &bd, NULL, &m_pCBallInOne ));
		DXUT_SetDebugName( m_pCBallInOne, "m_pCBallInOne");

		V_RETURN(D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"waterDrop.png", NULL, NULL, &m_pSparkTexSRV, NULL ));
		V_RETURN(D3DX11CreateShaderResourceViewFromFile( pd3dDevice, L"Flame.png", NULL, NULL, &m_pFlameTexSRV, NULL ));

		//Create random texture resource
		int iNumRandValues = 1024;
		srand( timeGetTime() );
		//create the data
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = new float[iNumRandValues * 4];
		if( !InitData.pSysMem )
			return E_OUTOFMEMORY;
		InitData.SysMemPitch = iNumRandValues * 4 * sizeof( float );
		InitData.SysMemSlicePitch = iNumRandValues * 4 * sizeof( float );
		for( int i = 0; i < iNumRandValues * 4; i++ )
		{
			( ( float* )InitData.pSysMem )[i] = float( ( rand() % 10000 ) - 5000 );
		}

		// Create the texture
		D3D11_TEXTURE1D_DESC dstex;
		dstex.Width = iNumRandValues;
		dstex.MipLevels = 1;
		dstex.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		dstex.Usage = D3D11_USAGE_DEFAULT;
		dstex.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = 0;
		dstex.ArraySize = 1;
		V_RETURN( pd3dDevice->CreateTexture1D( &dstex, &InitData, &m_pRandomTex1D ) );
		DXUT_SetDebugName( m_pRandomTex1D, "m_pRandomTex1D");
		SAFE_DELETE_ARRAY( InitData.pSysMem );

		// Create the resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
		SRVDesc.Format = dstex.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		SRVDesc.Texture2D.MipLevels = dstex.MipLevels;
		V_RETURN( pd3dDevice->CreateShaderResourceView( m_pRandomTex1D, &SRVDesc, &m_pRandomTexSRV ) );
		DXUT_SetDebugName( m_pRandomTexSRV, "m_pRandomTexSRV");

		//Create rendertaget resource
		D3D11_TEXTURE2D_DESC	RTtextureDesc = {0};
		RTtextureDesc.Width=m_rendertargetWidth;
		RTtextureDesc.Height=m_rendertargetHeight;
		RTtextureDesc.MipLevels=1;
		RTtextureDesc.ArraySize=1;
		RTtextureDesc.Format=DXGI_FORMAT_R16G16B16A16_FLOAT;
		RTtextureDesc.SampleDesc.Count=1;
		RTtextureDesc.Usage=D3D11_USAGE_DEFAULT;
		RTtextureDesc.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
		RTtextureDesc.CPUAccessFlags=0;
		RTtextureDesc.MiscFlags=0;

		V_RETURN(pd3dDevice->CreateTexture2D(&RTtextureDesc,NULL,&m_pOutputTexture2D));

		D3D11_SHADER_RESOURCE_VIEW_DESC RTshaderResourceDesc;
		RTshaderResourceDesc.Format=RTtextureDesc.Format;
		RTshaderResourceDesc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
		RTshaderResourceDesc.Texture2D.MostDetailedMip=0;
		RTshaderResourceDesc.Texture2D.MipLevels=1;
		V_RETURN(pd3dDevice->CreateShaderResourceView(m_pOutputTexture2D,&RTshaderResourceDesc,&m_pOutputTextureSRV));

		D3D11_RENDER_TARGET_VIEW_DESC	RTviewDesc;
		RTviewDesc.Format=RTtextureDesc.Format;
		RTviewDesc.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
		RTviewDesc.Texture2D.MipSlice=0;
		V_RETURN(pd3dDevice->CreateRenderTargetView(m_pOutputTexture2D,&RTviewDesc,&m_pOutputTextureRTV));


		//Create DepthStencil buffer and view
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth,sizeof(descDepth));
		descDepth.Width = m_rendertargetWidth;
		descDepth.Height = m_rendertargetHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format =DXUTGetDeviceSettings().d3d11.AutoDepthStencilFormat;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		hr = pd3dDevice->CreateTexture2D( &descDepth, NULL, &m_pOutputStencilTexture2D );


		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV,sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		V_RETURN( pd3dDevice->CreateDepthStencilView( m_pOutputStencilTexture2D, // Depth stencil texture
			&descDSV, // Depth stencil desc
			&m_pOutputStencilView ));  // [out] Depth stencil view

		// Create two depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc;

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		// Stencil test parameters
		dsDesc.StencilEnable = true;
		dsDesc.StencilReadMask = 0xFF;
		dsDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create depth stencil state
		pd3dDevice->CreateDepthStencilState(&dsDesc, &m_pDSsceneState);
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		pd3dDevice->CreateDepthStencilState(&dsDesc, &m_pDSparticleState);

		// Create the blend state
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC) );
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;        
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA         ;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE        ;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE    ;        ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE   ;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL ;
		//blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F ;
		V_RETURN( pd3dDevice->CreateBlendState( &blendDesc, &m_pTransparentOnBS ));
		blendDesc.RenderTarget[0].BlendEnable = false;
		V_RETURN( pd3dDevice->CreateBlendState( &blendDesc, &m_pTransparentOffBS ));

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory( &sampDesc, sizeof(sampDesc) );
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pGeneralTexSS ));

		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		V_RETURN(pd3dDevice->CreateSamplerState(&sampDesc, &m_pFlameTexSS ));

		m_RTviewport.Width=(float)m_rendertargetWidth;
		m_RTviewport.Height=(float)m_rendertargetHeight;
		m_RTviewport.MinDepth=0.0f;
		m_RTviewport.MaxDepth=1.0f;
		m_RTviewport.TopLeftX = 0;
		m_RTviewport.TopLeftY = 0;

		D3DXVECTOR3 vecEye( 0.0f, 0.0f, -50.0f );
		D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
		m_Camera.SetViewParams( &vecEye, &vecAt );
		m_Camera.SetRadius( 18.0f, 1.0f, 40.0f );

		return hr;
	}
	
	void Resize()
	{
		// Setup the camera's projection parameters
			float fAspectRatio = m_rendertargetWidth / ( FLOAT )m_rendertargetHeight;
			m_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 0.1f, 5000.0f );
			m_Camera.SetWindow(m_rendertargetWidth,m_rendertargetHeight );
			m_Camera.SetButtonMasks( MOUSE_RIGHT_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );
	}

	void UpdateParticles( ID3D11DeviceContext* pd3dImmediateContext )
	{
		// Set IA parameters
		ID3D11Buffer* pBuffers[1];
		if( m_bFirst )
			pBuffers[0] = m_pParticleStartVB;
		else
			pBuffers[0] = m_pParticleDrawFromVB;
		UINT stride[1] = { sizeof( PARTICLE_V ) };
		UINT offset[1] = { 0 };
		pd3dImmediateContext->IASetInputLayout( m_pParticlesVL );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, stride, offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

		// Point to the correct output buffer
		pBuffers[0] = m_pParticleStreamToVB;
		pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );

		pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->VSSetShader( m_pAdvanceParticleVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pAdvanceParticleGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( NULL, NULL, 0 );
		pd3dImmediateContext->GSSetShaderResources( 0, 1, &m_pRandomTexSRV );
		pd3dImmediateContext->GSSetSamplers(0,1,&m_pGeneralTexSS);


		if( m_bFirst )
			pd3dImmediateContext->Draw( 3, 0 );
		else
			pd3dImmediateContext->DrawAuto();


		// Get back to normal
		pBuffers[0] = NULL;
		pd3dImmediateContext->SOSetTargets( 1, pBuffers, offset );

		// Swap particle buffers
		ID3D11Buffer* pTemp = m_pParticleDrawFromVB;
		m_pParticleDrawFromVB = m_pParticleStreamToVB;
		m_pParticleStreamToVB = pTemp;

		m_bFirst = false;
	}

	void RenderParticles( ID3D11DeviceContext* pd3dImmediateContext )
	{
		UINT stride[1] = { sizeof( PARTICLE_V ) };
		UINT offset[1] = { 0 };
		pd3dImmediateContext->IASetInputLayout( m_pParticlesVL );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pParticleDrawFromVB, stride, offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
		pd3dImmediateContext->OMSetDepthStencilState( m_pDSparticleState, 0 );
		
		pd3dImmediateContext->RSSetViewports( 1, &m_RTviewport );
		pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS);
		pd3dImmediateContext->PSSetShaderResources( 1, 1, &m_pSparkTexSRV );
		pd3dImmediateContext->PSSetShaderResources( 2, 1, &m_pFlameTexSRV );
		pd3dImmediateContext->VSSetShader( m_pRenderParticleVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pRenderParticleGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pRenderParticlePS, NULL, 0 );
		pd3dImmediateContext->OMSetBlendState( m_pTransparentOnBS, NULL, 0xffffffff );
		pd3dImmediateContext->VSSetShaderResources( 0, 1, &m_pRandomTexSRV );
		pd3dImmediateContext->VSSetSamplers(0,1,&m_pGeneralTexSS);
		pd3dImmediateContext->PSSetSamplers(1,1,&m_pFlameTexSS);
		pd3dImmediateContext->DrawAuto();
		//pd3dImmediateContext->Draw( 3, 0 );
	}

	void RenderScene( ID3D11DeviceContext* pd3dImmediateContext )
	{
		SDKMESH_SUBSET* pSubset = NULL;
		D3D11_PRIMITIVE_TOPOLOGY PrimType;
		UINT Strides[1];
		UINT Offsets[1];
		ID3D11Buffer* pVB[1];
		pd3dImmediateContext->IASetInputLayout( m_pGeneralVL );    
		pd3dImmediateContext->RSSetViewports( 1, &m_RTviewport );
		pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
		pd3dImmediateContext->OMSetBlendState( m_pTransparentOffBS, NULL, 0xffffffff );
		pd3dImmediateContext->OMSetDepthStencilState( m_pDSsceneState, 1 );
		pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pRandomTexSRV );
		pd3dImmediateContext->PSSetSamplers(0,1,&m_pGeneralTexSS);

		pVB[0] = m_TeapotMesh.GetVB11( 0, 0 );
		Strides[0] = ( UINT )m_TeapotMesh.GetVertexStride( 0, 0 );
		Offsets[0] = 0;
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dImmediateContext->IASetIndexBuffer( m_TeapotMesh.GetIB11( 0 ), m_TeapotMesh.GetIBFormat11( 0 ), 0 );
		pd3dImmediateContext->VSSetShader( m_pTeapotVS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pGeneralMeshPS, NULL, 0 );
		for( UINT subset = 0; subset < m_TeapotMesh.GetNumSubsets( 0 ); ++subset )
		{
			pSubset = m_TeapotMesh.GetSubset( 0, subset );
			PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
			pd3dImmediateContext->IASetPrimitiveTopology( PrimType );
			pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
		}

		pVB[0] = m_SphereMesh.GetVB11( 0, 0 );
		Strides[0] = ( UINT )m_SphereMesh.GetVertexStride( 0, 0 );
		Offsets[0] = 0;
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
		pd3dImmediateContext->IASetIndexBuffer( m_SphereMesh.GetIB11( 0 ), m_SphereMesh.GetIBFormat11( 0 ), 0 );
		pd3dImmediateContext->VSSetShader( m_pSphereVS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pGeneralMeshPS, NULL, 0 );
		for( UINT subset = 0; subset < m_SphereMesh.GetNumSubsets( 0 ); ++subset )
		{
			pSubset = m_SphereMesh.GetSubset( 0, subset );
			PrimType = CDXUTSDKMesh::GetPrimitiveType11( ( SDKMESH_PRIMITIVE_TYPE )pSubset->PrimitiveType );
			pd3dImmediateContext->IASetPrimitiveTopology( PrimType );
			pd3dImmediateContext->DrawIndexed( ( UINT )pSubset->IndexCount, 0, ( UINT )pSubset->VertexStart );
		}

		// Draw ground
		UINT stride[1] = { sizeof( PARTICLE_V ) };
		UINT offset[1] = { 0 };
		pd3dImmediateContext->IASetInputLayout( m_pParticlesVL );
		pd3dImmediateContext->IASetVertexBuffers( 0, 1, &m_pParticleDrawFromVB, stride, offset );
		pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
		pd3dImmediateContext->GSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->PSSetConstantBuffers( 0, 1, &m_pCBallInOne );
		pd3dImmediateContext->VSSetShader( m_pGroundVS, NULL, 0 );
		pd3dImmediateContext->GSSetShader( m_pGroundGS, NULL, 0 );
		pd3dImmediateContext->PSSetShader( m_pGeneralMeshPS, NULL, 0 );
		pd3dImmediateContext->Draw( 1, 0 );
	}

	void Update( float fElapsedTime )
	{
		m_Camera.FrameMove( fElapsedTime );
	}

	void Render(ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime)
	{
		XMMATRIX m_Proj = (XMMATRIX)*m_Camera.GetProjMatrix();
		XMMATRIX m_View = (XMMATRIX)*m_Camera.GetViewMatrix();
		XMMATRIX m_World =(XMMATRIX)*m_Camera.GetWorldMatrix();

		XMVECTOR t;
		m_CBallInOne.mInvWorldView = XMMatrixTranspose(XMMatrixInverse(&t,m_View)); 
		m_CBallInOne.mWorldViewProj = XMMatrixTranspose( m_View*m_Proj );
		m_CBallInOne.mWorldView = XMMatrixTranspose( m_View);
		m_CBallInOne.fGlobalTime = fTime;
		m_CBallInOne.fElapsedTime = fElapsedTime;
		m_CBallInOne.mTeapot = XMMatrixTranspose(m_World*XMMatrixTranslation(-1.5,-0.5,-1.0));

		float x = ((float)( rand() % 10000 ))/5000.0f - 1.0f;
		float y = ((float)( rand() % 10000 ))/5000.0f - 1.0f;
		float z = ((float)( rand() % 10000 ))/5000.0f - 1.0f;
		float c = ((float)( rand() % 10000 ))/5000.0f - 1.0f;
		m_CBallInOne.vLightPos = XMFLOAT4( x*0.1,y*0.3,z*0.1,0);
		XMStoreFloat4(&m_CBallInOne.vLightCol, m_vLightColor*((c+1.0)*0.5*0.4+0.6));
		XMFLOAT4 vGravity( 0, 0.8, 0, 0 );		
		XMVECTOR vG = fElapsedTime * XMLoadFloat4( &vGravity );
		XMStoreFloat4( &(m_CBallInOne.vGravity), vG );
		pd3dImmediateContext->UpdateSubresource( m_pCBallInOne, 0, NULL, &m_CBallInOne, 0, 0 );
		UpdateParticles( pd3dImmediateContext);
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		pd3dImmediateContext->OMSetRenderTargets(1,&m_pOutputTextureRTV,m_pOutputStencilView);
		pd3dImmediateContext->ClearRenderTargetView( m_pOutputTextureRTV, ClearColor );
		pd3dImmediateContext->ClearDepthStencilView( m_pOutputStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		RenderScene( pd3dImmediateContext );
		RenderParticles( pd3dImmediateContext );
	}

	~SimpleFire()
	{

	}

	void Release()
	{
		SAFE_RELEASE(m_pRenderParticleVS);
		SAFE_RELEASE(m_pRenderParticleGS);
		SAFE_RELEASE(m_pRenderParticlePS);
		SAFE_RELEASE(m_pAdvanceParticleVS);
		SAFE_RELEASE(m_pAdvanceParticleGS);
		SAFE_RELEASE(m_pParticlesVL);
		SAFE_RELEASE(m_pParticleStartVB);
		SAFE_RELEASE(m_pParticleStreamToVB);
		SAFE_RELEASE(m_pParticleDrawFromVB);

		SAFE_RELEASE(m_pTeapotVS);
		SAFE_RELEASE(m_pSphereVS);
		SAFE_RELEASE(m_pGroundVS);
		SAFE_RELEASE(m_pGroundGS);
		SAFE_RELEASE(m_pGeneralMeshPS);
		SAFE_RELEASE(m_pGeneralVL);
		
		SAFE_RELEASE(m_pOutputTexture2D);
		SAFE_RELEASE(m_pOutputTextureRTV);
		SAFE_RELEASE(m_pOutputTextureSRV);
		SAFE_RELEASE(m_pOutputStencilTexture2D);
		SAFE_RELEASE(m_pOutputStencilView);
		SAFE_RELEASE(m_pTransparentOnBS);
		SAFE_RELEASE(m_pTransparentOffBS);
		SAFE_RELEASE(m_pDSparticleState);
		SAFE_RELEASE(m_pDSsceneState);

		SAFE_RELEASE(m_pSparkTexSRV);
		SAFE_RELEASE(m_pFlameTexSRV);

		SAFE_RELEASE(m_pRandomTexSRV);
		SAFE_RELEASE(m_pRandomTex1D);
		SAFE_RELEASE(m_pGeneralTexSS);
		SAFE_RELEASE(m_pFlameTexSS);
		SAFE_RELEASE(m_pCBallInOne);

		m_TeapotMesh.Destroy();
		m_SphereMesh.Destroy();

	}

	LRESULT HandleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
		switch(uMsg)
		{
		case WM_KEYDOWN:
			{
				int nKey = static_cast<int>(wParam);
				
				if (nKey == 'W')
				{
					m_CBallInOne.vSpherePos.z += 0.02;
					m_CBallInOne.mSphere = XMMatrixTranspose( XMMatrixTranslationFromVector(XMLoadFloat4(&m_CBallInOne.vSpherePos)));
				}
				if (nKey == 'S')
				{
					m_CBallInOne.vSpherePos.z -= 0.02;
					m_CBallInOne.mSphere = XMMatrixTranspose( XMMatrixTranslationFromVector(XMLoadFloat4(&m_CBallInOne.vSpherePos)));
				}
				if (nKey == 'A')
				{
					m_CBallInOne.vSpherePos.x -= 0.02;
					m_CBallInOne.mSphere = XMMatrixTranspose( XMMatrixTranslationFromVector(XMLoadFloat4(&m_CBallInOne.vSpherePos)));
				}
				if (nKey == 'D')
				{
					m_CBallInOne.vSpherePos.x += 0.02;
					m_CBallInOne.mSphere = XMMatrixTranspose( XMMatrixTranslationFromVector(XMLoadFloat4(&m_CBallInOne.vSpherePos)));
				}
				break;
			}
		}
		return 0;
	}
};