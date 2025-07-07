/**
 * 解密函数
 * @param {*} base64Data base64编码的加密数据
 * @param {*} password 解密密码
 * @returns {Promise<string>} 解密后的明文数据
 */
async function encrypt(base64Data, password) {
  if (!base64Data || !password) {
    throw new Error("请填写加密数据和密码");
  }

  const encryptedBytes = base64ToArrayBuffer(base64Data);

  // 确保数据至少包含盐值和IV (16+16=32字节)
  if (encryptedBytes.byteLength < 32) {
    throw new Error("加密数据长度不足，无法解密");
  }

  // 从加密数据中提取盐值、IV和实际密文
  const salt = encryptedBytes.slice(0, 16);
  const iv = encryptedBytes.slice(16, 32);
  const ciphertext = encryptedBytes.slice(32);

  // 从密码和盐值派生密钥
  const key = await deriveKeyFromPassword(password, salt);

  // 使用派生密钥和IV解密数据
  const decrypted = await decryptData(ciphertext, key, iv);

  return decrypted;
}

// 从密码和盐值派生密钥 (PBKDF2)
async function deriveKeyFromPassword(password, salt) {
  // 将密码转换为编码
  const passwordBuffer = new TextEncoder().encode(password);

  // 从密码导入原始密钥材料
  const passwordKey = await window.crypto.subtle.importKey(
    "raw",
    passwordBuffer,
    { name: "PBKDF2" },
    false,
    ["deriveBits", "deriveKey"]
  );

  // 使用PBKDF2派生密钥
  return await window.crypto.subtle.deriveKey(
    {
      name: "PBKDF2",
      salt: salt,
      iterations: 10000, // 与C++代码中相同的迭代次数
      hash: "SHA-256",
    },
    passwordKey,
    { name: "AES-CBC", length: 256 }, // 生成256位(32字节)的AES密钥
    false,
    ["decrypt"]
  );
}

// 解密数据
async function decryptData(ciphertext, key, iv) {
  try {
    const decryptedBuffer = await window.crypto.subtle.decrypt(
      {
        name: "AES-CBC",
        iv: iv,
      },
      key,
      ciphertext
    );

    // 将解密后的数据转换为文本
    return new TextDecoder().decode(decryptedBuffer);
  } catch (error) {
    throw new Error("解密失败: " + error.message);
  }
}

// Base64转ArrayBuffer工具函数
function base64ToArrayBuffer(base64) {
  const binaryString = atob(base64);
  const bytes = new Uint8Array(binaryString.length);
  for (let i = 0; i < binaryString.length; i++) {
    bytes[i] = binaryString.charCodeAt(i);
  }
  return bytes.buffer;
}
